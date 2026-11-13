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
#include <iterator>
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
#include "EBGeometry_Domain.hpp"
#include "EBGeometry_GPUTransform.hpp"
#include "EBGeometry_SurfacePieces.hpp"
#include "EBGeometry_Viewer.hpp"

#ifdef EBGEOMETRY_VIEWER_SDL2
#include "EBGeometry_ViewerFont.hpp"
#endif

namespace EBGeometry::Render {

namespace ViewerDetail {

/**
 * @brief Surface groups, as a setup to edit (the viewer's starting point when the settings give one).
 * @param[in] a_setup Surface groups resolving on the host.
 * @return Its surface groups and selectors; no subdomains.
 */
inline DomainSetup
setupOf(const SurfaceGroups& a_setup)
{
  DomainSetup setup;

  for (uint32_t g = 0; g < a_setup.getNumSurfaceGroups(); g++) {
    (void)setup.addSurfaceGroup();
  }

  for (uint32_t i = 0; i < a_setup.getNumSelectors(); i++) {
    setup.add(a_setup.getSurfaceGroupOfSelector(i), a_setup.getSelector(i));
  }

  return setup;
}

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
   * @brief The key or keys, as written for a reader; null for a heading over the keys after it (m_what).
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
   * @brief The title in the row of menus, short.
   */
  const char* m_short;

  /**
   * @brief The keys, in the order they are listed, with headings (see Key::m_keys).
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
  std::vector<KeyGroup> groups{
    {"Camera and light",
     "Camera",
     {{nullptr, "Camera"},
      {"Drag", "Orbit the camera"},
      {"Ctrl + drag", "Pan the camera"},
      {"Wheel", "Zoom"},
      {"Double-click", "Center the camera on the point under the cursor"},
      {"1, 2, 3", "Look from the front, the side, the top"},
      {"O", "Switch between perspective and orthographic"},
      {"+, -", "Narrower or wider field of view"},
      {"R", "Reset the camera, the light, what is picked and what is shown"},
      {nullptr, "Light"},
      {"Right drag", "Move the light"},
      {"Left, Right", "Weaker or stronger highlights"},
      {"Shift + Left, Right", "Broader or tighter highlights"}}},
    {"Rendering",
     "Rendering",
     {{"M, Shift + M", "Next or previous view: shaded, x-ray, costs"},
      {"P", "Next coloring: by normal, one color, by object"},
      {"S", "Shadows on or off"},
      {"A", "Ambient occlusion on or off"},
      {"F", "Floor on or off"},
      {"U", "Axis planes through the origin: all three, the one facing you most, or none"},
      {"B", "Sky on or off"},
      {"Shift + Up, Down", "More or fewer rays per pixel (1 to 64)"}}},
    {"Section and contours",
     "Section",
     {{"X", "Cut across x, then y, then z"},
      {"Ctrl + X", "Remove the cut"},
      {"Shift + X", "See through the cut face, or make it opaque"},
      {"N", "Reverse the cut (keep the other side)"},
      {"Up, Down", "Move the plane in coarse steps"},
      {"Z", "Slice view: the plane flat on screen (wheel moves it, drag pans); Z again leaves"},
      {"Ctrl + wheel", "Move the plane in fine steps"},
      {"I", "Lines of equal distance: off, as set, coarse, fine, finer"}}},
    {"Picking and measuring",
     "Picking",
     {{"Click", "Pick what is under the cursor (the background clears it)"},
      {"Shift + wheel", "Step through the surfaces under the cursor, front to back"},
      {"T", "See through the picked surface, or make it opaque again"},
      {"Shift + T", "Make every surface opaque again"},
      {"Shift + click", "Measure between two points (the background clears it)"},
      {"Right click",
       "Probe a point: where it is, its region and functions (the background "
       "clears it)"}}},
    {"Window",
     "Window",
     {{"Tab, Shift + Tab", "Next or previous menu"},
      {"F1 to F6", "Show a menu"},
      {"H", "Hide or show the menu"},
      {"D", "Show or hide details (numbers, coordinates, statistics)"},
      {"C", "Print the camera and settings as C++"},
      {"W", "Save the full frame as a PPM image"},
      {"Esc, Q", "Close the window"}}}};

  if (a_device) {
    groups[1].m_keys.push_back({"G", "Render on the GPU or the CPU"});
  }

  // A shape whose regions can be shown and hidden (see RegionView) gets a group of its own, before the window's, and
  // the section's keys for its domains.
  if (a_regions) {
    groups[2].m_keys.push_back({"E", "Show the domain's functions, each subdomain's, or the regions'"});
    groups[2].m_keys.push_back({"Shift + E", "Lines where the domain shown does not compute, or none"});

    groups.insert(groups.end() - 1,
                  KeyGroup{"Regions",
                           "Regions",
                           {{nullptr, "Coloring and showing"},
                            {"K", "Color by the next kind (the panel's Color by sets one)"},
                            {"L", "Show only surfaces like the picked one, or everything again"},
                            {"V", "Show one region at a time, in turn, then all"},
                            {"Shift + V", "Hide the picked region"},
                            {"Ctrl + V", "Show every region"},
                            {nullptr, "Surface groups (as the panel's buttons)"},
                            {"Ctrl + click", "Pick several: add one, or take it out again"},
                            {"Enter", "Add what is picked to the current surface group"},
                            {"Shift + Enter", "Add the whole picked surfaces"},
                            {"Ctrl + Enter", "Add the interfaces between the picked pieces' regions"},
                            {"Backspace", "Remove the picked pieces from the current surface group"},
                            {"J", "Next surface group (past the last, a new one)"},
                            {"Delete", "Delete the current surface group"}}});
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
 * @brief A cost in a cost view's unit, short: "850 ns", "2.5 us", "1.2 ms"; "980 cycles", "12k cycles", "3.4M cycles".
 * @details Internal helper; not part of the public API.
 * @param[in] a_cost The cost.
 * @param[in] a_unit Its unit.
 * @param[in] a_unitText Whether to append the unit.
 * @return The text.
 */
inline std::string
costText(const double a_cost, const CostUnit a_unit, const bool a_unitText = true)
{
  // One decimal below ten, unless it is zero.
  const auto shortNumber = [](const double a_value) {
    char text[32];

    const bool whole = a_value >= 9.95 || std::abs(a_value - std::round(a_value)) < 0.05;

    std::snprintf(text, sizeof(text), whole ? "%.0f" : "%.1f", a_value);

    return std::string(text);
  };

  // The next unit from where the number would round to 1000 ("999.6" is "1 us", not "1000 ns").
  constexpr double Next = 999.5;

  if (a_unit == CostUnit::Nanoseconds) {
    return (a_cost >= 1e3 * Next) ? shortNumber(1e-6 * a_cost) + (a_unitText ? " ms" : "ms")
           : (a_cost >= Next)     ? shortNumber(1e-3 * a_cost) + (a_unitText ? " us" : "us")
                                  : shortNumber(a_cost) + (a_unitText ? " ns" : "ns");
  }

  const std::string number = (a_cost >= 1e6 * Next)   ? shortNumber(1e-9 * a_cost) + "G"
                             : (a_cost >= 1e3 * Next) ? shortNumber(1e-6 * a_cost) + "M"
                             : (a_cost >= Next)       ? shortNumber(1e-3 * a_cost) + "k"
                                                      : shortNumber(a_cost);

  return a_unitText ? number + " cycles" : number;
}

/**
 * @brief Rays per second, short: "41 Mrays/s", "2.3 Grays/s".
 * @details Internal helper; not part of the public API.
 * @param[in] a_raysPerSecond The rate.
 * @return The text.
 */
inline std::string
rateText(const double a_raysPerSecond)
{
  const bool   giga  = a_raysPerSecond >= 1e9;
  const double value = giga ? 1e-9 * a_raysPerSecond : 1e-6 * a_raysPerSecond;

  char text[48];

  std::snprintf(text, sizeof(text), (value < 9.95) ? "%.1f %s" : "%.0f %s", value, giga ? "Grays/s" : "Mrays/s");

  return std::string(text);
}

/**
 * @brief A cost view's colorbar, on the overlay's dark panel: a title (the unit and the rays per second), the scale's
 * colors (costColor()) and labeled ticks at round costs (1, 2 and 5 times a power of ten), the ends reading "<=" the
 * cheapest and ">=" the dearest color's cost.
 * @details Internal helper; not part of the public API.
 * @param[in]  a_stats    The frame's Stats, from a cost view.
 * @param[in]  a_onDevice Whether the frame was rendered on a device.
 * @param[in]  a_scale    Pixels per glyph pixel.
 * @param[out] a_pixels   The colorbar, RGBA (0xRRGGBBAA, row by row).
 * @param[out] a_width    Its width.
 * @param[out] a_height   Its height.
 */
inline void
drawColorbar(const Stats&           a_stats,
             const bool             a_onDevice,
             const int              a_scale,
             std::vector<uint32_t>& a_pixels,
             int&                   a_width,
             int&                   a_height)
{
  // The strip and the ticks' labels span this many text columns.
  constexpr int Columns = 44;

  const std::string per = (a_stats.m_costUnit == CostUnit::Cycles) ? "Cycles per ray, waits included" : "Time per ray";

  const std::string rate = ", " + rateText(a_stats.m_raysPerSecond) + " on the " + (a_onDevice ? "GPU" : "CPU");

  // The labels, placed left to right where they fit: the two ends first, then the round costs between them.
  std::string         labels(Columns, ' ');
  std::vector<double> ticks;

  const auto columnOf = [&](const double a_cost) {
    return static_cast<int>(std::lround(costPlace(a_stats, a_cost) * double(Columns - 1)));
  };
  const auto fits = [&](const int a_from, const std::size_t a_length) {
    if (a_from < 0 || a_from + static_cast<int>(a_length) > Columns) {
      return false;
    }

    // Two blank columns on either side.
    for (int c = Math::max(0, a_from - 2); c < Math::min(Columns, a_from + static_cast<int>(a_length) + 2); c++) {
      if (labels[std::size_t(c)] != ' ') {
        return false;
      }
    }

    return true;
  };

  const std::string low  = "<=" + costText(a_stats.m_costLow, a_stats.m_costUnit, false);
  const std::string high = ">=" + costText(a_stats.m_costHigh, a_stats.m_costUnit, false);

  labels.replace(0, low.size(), low);

  // The two ends' labels are at most 10 characters each, so both fit in the strip's columns.
  if (fits(Columns - static_cast<int>(high.size()), high.size())) {
    labels.replace(std::size_t(Columns) - high.size(), high.size(), high);
  }

  if (a_stats.m_costLow > 0.0 && a_stats.m_costHigh > a_stats.m_costLow) {
    for (double decade = std::pow(10.0, std::floor(std::log10(a_stats.m_costLow))); decade <= a_stats.m_costHigh;
         decade *= 10.0) {

      for (const double step : {1.0, 2.0, 5.0}) {
        const double cost = step * decade;

        if (cost <= a_stats.m_costLow || cost >= a_stats.m_costHigh) {
          continue;
        }

        const std::string text   = costText(cost, a_stats.m_costUnit, false);
        const int         column = columnOf(cost);
        const int         from   = column - static_cast<int>(text.size()) / 2;

        if (fits(from, text.size())) {
          labels.replace(std::size_t(from), text.size(), text);
          ticks.push_back(cost);
        }
      }
    }
  }

  drawText({per + rate, std::string(Columns, ' '), labels}, a_scale, a_pixels, a_width, a_height);

  // The strip in the second line, a column of pixels per place on the scale, and a mark under it at each labeled tick.
  const int margin = 6 * a_scale;
  const int top    = margin + GlyphHeight * a_scale + a_scale;
  const int bottom = margin + 2 * GlyphHeight * a_scale - 3 * a_scale;
  const int width  = Columns * GlyphWidth * a_scale;

  for (int x = 0; x < width; x++) {
    const uint32_t color = costColor(double(x) / double(Math::max(1, width - 1)));

    for (int y = top; y < bottom; y++) {
      a_pixels[std::size_t(y) * std::size_t(a_width) + std::size_t(margin + x)] = color;
    }
  }

  // A mark at its cost's own place on the strip (its label is centered on the nearest text column).
  for (const double cost : ticks) {
    const int x = margin + static_cast<int>(std::lround(costPlace(a_stats, cost) * double(width - 1)));

    for (int y = bottom; y < bottom + 3 * a_scale; y++) {
      for (int dx = 0; dx < a_scale; dx++) {
        a_pixels[std::size_t(y) * std::size_t(a_width) + std::size_t(x + dx)] = 0xF0F0F0FFU;
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
 * @brief What a click on a button of the viewer's panel (the picked piece, the regions, the surface groups) does.
 * @details Internal helper; not part of the public API.
 */
enum class PanelAction : uint8_t
{
  None,           ///< Nothing.
  AddPicked,      ///< Add what is picked (a part of a surface, or a region's surfaces) to the current surface group.
  AddPiece,       ///< Add the picked piece, all its parts.
  AddSurface,     ///< Add the picked piece's whole surface.
  AddInterface,   ///< Add the interface between the picked piece's two regions.
  AddRegion,      ///< Add every surface of the picked piece's region (the one inside).
  AddRest,        ///< Add the remaining surfaces (those no other entry holds) to the current surface group.
  RemovePiece,    ///< Remove what holds the picked piece from the current surface group.
  Unpick,         ///< Take one out of what is picked.
  UnpickAll,      ///< Pick nothing.
  MakeOpaque,     ///< Undo what else hides or fades surfaces: keys seen through, one shown alone, groups' states.
  PickMode,       ///< Pick the part of a surface clicked, or the region there.
  ToggleRegion,   ///< Show or hide a region.
  CurrentGroup,   ///< Make a surface group the one added to.
  DeleteGroup,    ///< Delete a surface group.
  RemoveSelector, ///< Remove a selector.
  NewGroup,       ///< Start a surface group.
  NextUnassigned, ///< Pick the next piece in no surface group.
  ListPieces,     ///< Show or hide the list of pieces.
  PickPiece,      ///< Pick a piece of the list.
  ColorBy,        ///< Color by a kind of key, the normal, or one color (see colorChoices).
  RenameGroup,    ///< Type a surface group's name.
  Details,        ///< Show or hide details (numbers, coordinates, statistics).
  HideGroup,      ///< Cycle a surface group's surfaces: shown, translucent, hidden, shown again.
  CopyGroup,      ///< Copy a surface group: a new one with the same selectors.
  SelectDomain,   ///< Make a registered domain the active one and highlight it, or stop highlighting it.
  HighlightSubdomain, ///< Highlight a subdomain's surfaces, or stop.
  HideSubdomain,      ///< Hide a subdomain's regions, or show them again.
  OnlySubdomain,      ///< Show only a subdomain's regions.
  HideDomain,         ///< Hide the active domain's regions.
  OnlyDomain,         ///< Show only the active domain's regions.
  ShowAllRegions,     ///< Show every region again.
  HighlightRegion     ///< Highlight a region's surfaces (those with it on either side), or stop.
};

/**
 * @brief A line of the viewer's overlay or panel, leading with a capital letter: the first letter past its
 * indentation and its markers ("> ", "[x] ", "[~] ", "[ ] ") is capitalized.
 * @details Internal helper; not part of the public API.
 * @param[in] a_line The line.
 * @return The line.
 */
inline std::string
sentenceCase(std::string a_line)
{
  std::size_t i = a_line.find_first_not_of(' ');

  if (i == std::string::npos) {
    return a_line;
  }

  for (const char* marker : {"> ", "[x] ", "[~] ", "[ ] "}) {
    if (a_line.compare(i, std::strlen(marker), marker) == 0) {
      i += std::strlen(marker);
    }
  }

  if (i < a_line.size() && a_line[i] >= 'a' && a_line[i] <= 'z') {
    a_line[i] = static_cast<char>(a_line[i] - 'a' + 'A');
  }

  return a_line;
}

/**
 * @brief What the viewer's panel offers to color a view of regions by: a kind of key (RegionColoring), or the normal,
 * or one color.
 * @details Internal helper; not part of the public API.
 */
struct ColorChoice
{
  /**
   * @brief Its button's label.
   */
  const char* m_label;

  /**
   * @brief The render coloring.
   */
  ColorBy m_colorBy;

  /**
   * @brief The kind of key, for ColorBy::Primitive.
   */
  RegionColoring m_coloring;
};

/**
 * @brief The colorings the viewer's panel offers, in its order.
 * @details Internal helper; not part of the public API.
 */
inline constexpr ColorChoice colorChoices[] = {{"Material", ColorBy::Primitive, RegionColoring::Material},
                                               {"Region", ColorBy::Primitive, RegionColoring::Region},
                                               {"Surface group", ColorBy::Primitive, RegionColoring::SurfaceGroup},
                                               {"Surface label", ColorBy::Primitive, RegionColoring::SurfaceLabel},
                                               {"Surface", ColorBy::Primitive, RegionColoring::Surface},
                                               {"Part", ColorBy::Primitive, RegionColoring::Part},
                                               {"Interface", ColorBy::Primitive, RegionColoring::Interface},
                                               {"Normal", ColorBy::Normal, RegionColoring::Region},
                                               {"One color", ColorBy::Uniform, RegionColoring::Region}};

/**
 * @brief A clickable span of a line of the viewer's panel: columns [m_begin, m_end) and what a click there does.
 * @details Internal helper; not part of the public API.
 */
struct PanelSpan
{
  /**
   * @brief The first column.
   */
  std::size_t m_begin = 0;

  /**
   * @brief One past the last column.
   */
  std::size_t m_end = 0;

  /**
   * @brief What a click does.
   */
  PanelAction m_action = PanelAction::None;

  /**
   * @brief Its argument: a region, a surface group, a selector.
   */
  uint32_t m_value = 0;
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

  // Text input only while a name is typed (some platforms start with it on).
  SDL_StopTextInput();

  std::printf("EBGeometry viewer keys\n");

  for (const KeyGroup& group : keys(a_deviceShape != nullptr, hasRegionVisibility<S>)) {
    std::printf("\n%s\n", group.m_title);

    for (const Key& key : group.m_keys) {
      if (key.m_keys == nullptr) {
        std::printf("  %s\n", key.m_what);
      }
      else {
        std::printf("    %-20s %s\n", key.m_keys, key.m_what);
      }
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

  // The surface group that Enter adds to (see RegionSettings::m_surfaceGroups).
  uint32_t currentGroup = 0;

  // The surface groups as edited (the active domain's, or those of the settings given), and as last shown: surface groups
  // built from them in a small pool of its own, and mirrored for a device, after each change (publishGroups()). Those
  // pools live as long as the viewer, so that a render in the background never outlives the surface groups it reads: each
  // change keeps a pool of a few selectors, and its mirror on a device, until the viewer closes.
  DomainSetup                        edited = ViewerDetail::setupOf(a_settings.m_regions.m_surfaceGroups);
  DomainSetup                        published;
  bool                               publishedOnce = false;
  SurfaceGroups                      deviceGroups;
  std::vector<std::unique_ptr<Pool>> groupPools;

  // The surface group whose name is being typed (SurfaceGroups::None for none), and the name so far: while it is, keys type
  // it instead of acting.
  uint32_t    renaming = SurfaceGroups::None;
  std::string renameText;

  // The pieces of surface of a view of regions, found once, the last one picked from them, and whether the panel
  // lists them.
  std::vector<SurfacePiece<T>> pieces;
  std::size_t                  pieceIndex = std::size_t(-1);
  bool                         listPieces = false;

  if constexpr (hasRegionVisibility<S>) {
    pieces = findSurfacePieces(a_shape.getTape());
  }

  // The names of the regions, materials, surfaces and groups, as the program gave them.
  RegionNames names = (a_names != nullptr) ? *a_names : RegionNames();

  // The pieces the object may have (from the index: a superset), each with the one found for it, if any (npos); a
  // piece found that the index does not list (names not built from an index) is added, so that nothing found is left
  // out.
  const SurfaceInventory&    inventory = names.getInventory();
  std::vector<PossiblePiece> possible  = possiblePieces(inventory);
  std::vector<std::size_t>   foundOf(possible.size(), std::size_t(-1));
  std::vector<bool>          listed(pieces.size(), false);

  for (std::size_t c = 0; c < possible.size(); c++) {
    for (std::size_t f = 0; f < pieces.size(); f++) {
      const SurfaceRef<T>& s = pieces[f].m_surface;

      if (s.m_surface == possible[c].m_surface && s.m_ownerRegion == possible[c].m_ownerRegion &&
          s.m_neighborRegion == possible[c].m_neighborRegion) {
        foundOf[c] = f;
        listed[f]  = true;
      }
    }
  }

  for (std::size_t f = 0; f < pieces.size(); f++) {
    if (!listed[f]) {
      const SurfaceRef<T>& s = pieces[f].m_surface;

      possible.push_back(PossiblePiece{s.m_surface, s.m_ownerRegion, s.m_neighborRegion});
      foundOf.push_back(f);
    }
  }

  // How the surface groups cover each possible piece, kept until they change. A piece the inventory does not hold is
  // judged by the surface where it was found.
  std::vector<PieceCoverage> coverage;
  DomainSetup                coveredGroups;
  bool                       coverageKnown = false;

  const auto coverageOfGroups = [&](const DomainSetup& a_setup, const std::size_t a_piece) {
    const PossiblePiece& piece = possible[a_piece];

    if (piece.m_surface < inventory.getNumSurfaces()) {
      return coverageOf(piece, inventory, a_setup);
    }

    PieceCoverage judged;

    judged.m_surfaceGroup = a_setup.surfaceGroupOf(pieces[foundOf[a_piece]].m_surface);
    judged.m_coverage     = (judged.m_surfaceGroup != SurfaceGroups::None) ? Coverage::Full : Coverage::None;

    return judged;
  };

  const auto coverages = [&]() -> const std::vector<PieceCoverage>& {
    const DomainSetup& setup = edited;

    if (!coverageKnown || setup != coveredGroups) {
      coverage.clear();

      for (std::size_t c = 0; c < possible.size(); c++) {
        coverage.push_back(coverageOfGroups(setup, c));
      }

      coveredGroups = setup;
      coverageKnown = true;
    }

    return coverage;
  };

  // Start typing a surface group's name, or stop: keeping what was typed (unless nothing was), or not.
  const auto startRename = [&](const uint32_t a_group) {
    renaming = a_group;

    renameText.clear();

    SDL_StartTextInput();
  };

  const auto finishRename = [&](const bool a_keep) {
    if (renaming != SurfaceGroups::None && a_keep && !renameText.empty()) {
      names.setSurfaceGroup(renaming, renameText);
    }

    renaming = SurfaceGroups::None;

    renameText.clear();

    SDL_StopTextInput();
  };

  // The colors the names give the keys of the coloring shown, over the settings' own (see RegionNames::setColor()).
  const auto applyColors = [&]() {
    if constexpr (hasRegionVisibility<S>) {
      for (uint32_t i = 0; i < Settings<T>::PrimitiveColors; i++) {
        Vec3T<double> color;

        settings.m_primitiveColors[i] = names.colorOf(settings.m_regions.m_coloring, i, color)
                                          ? Vec3T<T>(T(color[0]), T(color[1]), T(color[2]))
                                          : a_settings.m_primitiveColors[i];
        settings.m_materialColors[i]  = names.colorOf(RegionColoring::Material, i, color)
                                          ? Vec3T<T>(T(color[0]), T(color[1]), T(color[2]))
                                          : a_settings.m_materialColors[i];
      }
    }
  };

  // With domains registered, the settings show the active one's surface groups (edited there, and kept with the
  // domain when another is made active).
  if constexpr (hasRegionVisibility<S>) {
    if (names.getNumDomains() > 0U) {
      edited = names.getDomain(names.getActiveDomain());
    }
  }

  applyColors();

  // The color a key is drawn in (as the renderer picks it, before shading), as an RGBA pixel for a swatch.
  const auto swatchOf = [&](const uint32_t a_key) {
    const Vec3T<T> linear = (a_key == Settings<T>::NoPrimitive) ? Vec3T<T>(T(0.6), T(0.6), T(0.6))
                            : (a_key < Settings<T>::PrimitiveColors && settings.m_primitiveColors[a_key][0] >= T(0))
                              ? settings.m_primitiveColors[a_key]
                              : RenderDetail::palette<T>(a_key);

    uint32_t pixel = 0xFFU;

    for (int k = 0; k < 3; k++) {
      const T g = std::pow(Math::clamp(linear[k], T(0), T(1)), T(1) / T(2.2));

      pixel |= uint32_t(Math::clamp(g * T(255) + T(0.5), T(0), T(255))) << (24 - 8 * k);
    }

    return pixel;
  };

  // A surface group's selector, in words and as C++.
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

  // The panel of a view of regions (the picked surface, the regions, the surface groups), drawn over the image's top
  // left corner: its
  // lines, and what a click on each does.
  std::vector<std::string>            panelLines;
  std::vector<std::vector<PanelSpan>> panelActions;
  std::vector<uint32_t>               panelSwatches;
  SDL_Texture*                        panel       = nullptr;
  int                                 panelWidth  = 0;
  int                                 panelHeight = 0;
  int                                 panelScale  = 1;
  std::vector<uint32_t>               panelPixels;
  const T                             diag  = (box.getHighCorner() - box.getLowCorner()).length();
  const T                             scale = (std::isfinite(diag) && diag > T(0)) ? diag : camera.m_distance;

  // The section plane: off (0), as the settings give it (1, only when they turn it on), or across the x, y
  // or z axis through the box's center (2 to 4); moved along its normal by an offset, and reversed (the
  // other side removed) when flipped.
  const bool     givenSection  = a_settings.m_section;
  const Vec3T<T> center        = (std::isfinite(diag) && diag > T(0)) ? box.getCentroid() : camera.m_target;
  int            sectionMode   = givenSection ? 1 : 0;
  bool           sectionFlip   = false;
  T              sectionOffset = T(0);

  const char* sectionNames[] = {"Off", "As given", "Across x", "Across y", "Across z"};

  // The slice view (Z): the section plane flat on screen, seen square from the removed side by an orthographic camera,
  // without the floor and shadows; the wheel moves the plane and a drag pans. The camera and the settings it changes,
  // as they were, to go back to.
  bool      slicing        = false;
  Camera<T> beforeSlice    = camera;
  bool      sliceFloor     = settings.m_floor;
  bool      sliceShadows   = settings.m_shadows;
  bool      sliceOcclusion = settings.m_occlusion;

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

  // What a click picks in a view of regions: the part of a surface clicked (a cylinder's cap, a box's face), or the
  // region seen there, every surface of it.
  enum class PickMode : uint8_t
  {
    Surface,
    Region
  };

  PickMode pickMode = PickMode::Surface;

  // What is picked: each the piece clicked and what the pick mode picks of it. A click picks one, Ctrl and a click
  // adds one or takes it out again; they are highlighted together, and the panel's buttons add or remove them.
  struct Picked
  {
    SurfaceRef<T>   m_piece;
    SurfaceSelector m_selector;
  };

  std::vector<Picked> selection;

  // What the pick mode picks of a piece: its part, or the region on its side (the one inside, not the background).
  const auto selectorOf = [&](const SurfaceRef<T>& a_piece) {
    if (pickMode == PickMode::Region) {
      return SurfaceSelector::region((a_piece.m_ownerRegion != 0U) ? a_piece.m_ownerRegion : a_piece.m_neighborRegion);
    }

    return SurfaceSelector::piece(a_piece).withPart(a_piece.m_part);
  };

  const auto sameSelector = [](const SurfaceSelector& a_first, const SurfaceSelector& a_second) {
    return a_first.m_surface == a_second.m_surface && a_first.m_part == a_second.m_part &&
           a_first.m_regionA == a_second.m_regionA && a_first.m_regionB == a_second.m_regionB &&
           a_first.m_label == a_second.m_label && a_first.m_materialA == a_second.m_materialA &&
           a_first.m_materialB == a_second.m_materialB && a_first.m_rest == a_second.m_rest;
  };

  // The highlight the render shows: a highlighted surface group's or subdomain's surfaces; else what is picked. The
  // selection is drawn only while nothing else is highlighted.
  const auto showHighlight = [&]() {
    if constexpr (hasRegionVisibility<S>) {
      RegionSettings& regions = settings.m_regions;

      const bool other = regions.m_highlightGroup != RegionSettings::None || regions.m_highlightRegions != 0U;

      regions.m_numSelected = 0U;

      if (!other) {
        for (const Picked& entry : selection) {
          if (regions.m_numSelected < RegionSettings::MaxSelected) {
            regions.m_selected[regions.m_numSelected++] = entry.m_selector;
          }
        }
      }

      settings.m_highlightPrimitive = other                          ? RegionSettings::Highlighted
                                      : (regions.m_numSelected > 0U) ? RegionSettings::Selected
                                      : picked                       ? pick.m_primitive
                                                                     : Settings<T>::NoPrimitive;
      settings.m_highlightCount     = (!other && regions.m_numSelected > 0U) ? regions.m_numSelected : 1U;
    }
  };

  const auto clearSelection = [&]() {
    selection.clear();

    showHighlight();
  };

  // The picked surface's key under the region settings now (after the coloring or the surface groups changed): a short ray
  // into the picked point, along its normal, asks the view again; the highlight follows.
  const auto rekeyPick = [&]() {
    if constexpr (hasRegionVisibility<S>) {
      if (picked) {
        const BoundingVolumes::AABBT<T> bounds = shape.computeBoundingVolume();
        const T                         size   = (bounds.getHighCorner() - bounds.getLowCorner()).length();
        const T step = (size > T(0) && size < Math::Limits<T>::max()) ? T(1e-3) * size : T(1e-3);

        // The key as the coloring gives it, not as a highlighted surface group's.
        Settings<T> keyed = settings;

        keyed.m_regions.m_highlightGroup   = RegionSettings::None;
        keyed.m_regions.m_highlightRegions = 0U;
        keyed.m_regions.m_numSelected      = 0U;

        const Vec3T<T> n   = pick.m_normal / Math::max(pick.m_normal.length(), Math::Limits<T>::min());
        const HitT<T>  hit = RenderDetail::shownAs(shape, keyed)
                              .firstHit(RayT<T>{pick.m_point + step * n, -n}, T(0), T(2) * step, RayOptions<T>());

        picked = hit.hit();

        if (picked) {
          pick.m_primitive = hit.m_primitive;
        }
      }

      showHighlight();
    }
  };

  // What each way of adding a picked piece adds: the piece, its whole surface, every surface of its region (the one
  // inside), or everything between its two regions.
  const auto selectorFor = [](const PanelAction a_action, const SurfaceRef<T>& a_piece) {
    switch (a_action) {
    case PanelAction::AddSurface: {
      return SurfaceSelector::surface(a_piece.m_surface);
    }
    case PanelAction::AddRegion: {
      return SurfaceSelector::region(a_piece.m_ownerRegion);
    }
    case PanelAction::AddInterface: {
      return SurfaceSelector::interface(a_piece.m_ownerRegion, a_piece.m_neighborRegion);
    }
    case PanelAction::AddRest: {
      return SurfaceSelector::rest();
    }
    default: {
      return SurfaceSelector::piece(a_piece);
    }
    }
  };

  // Whether a possible piece is one the active domain must tag: it touches the domain's regions (every piece, with no
  // domain registered).
  const auto relevant = [&](const std::size_t a_piece) {
    return names.getNumDomains() == 0U ||
           names.getDomain(names.getActiveDomain())
             .touches(possible[a_piece].m_ownerRegion, possible[a_piece].m_neighborRegion);
  };

  // The drawn regions (1 to 63) of a subdomain of the active domain, a bit each; of all its subdomains (None).
  const auto subdomainRegions = [&](const uint32_t a_subdomain) {
    uint64_t mask = 0U;

    if constexpr (hasRegionVisibility<S>) {
      if (names.getNumDomains() > 0U) {
        const DomainSetup& subdomains = names.getDomain(names.getActiveDomain());

        for (uint32_t k = 1; k < Math::min(shape.getNumRegions(), 64U); k++) {
          const uint32_t d = subdomains.subdomainOf(k);

          mask |= (d != Subdomains::None && (a_subdomain == Subdomains::None || d == a_subdomain)) ? (uint64_t(1) << k)
                                                                                                   : uint64_t(0);
        }
      }
    }
    else {
      (void)a_subdomain;
    }

    return mask;
  };

  // All the regions (0 to 63, the background included) of a subdomain of the active domain, a bit each: its field.
  const auto subdomainAllRegions = [&](const uint32_t a_subdomain) {
    uint64_t mask = 0U;

    if constexpr (hasRegionVisibility<S>) {
      if (names.getNumDomains() > 0U) {
        const DomainSetup& subdomains = names.getDomain(names.getActiveDomain());

        for (uint32_t k = 0; k < Math::min(shape.getNumRegions(), 64U); k++) {
          mask |= (subdomains.subdomainOf(k) == a_subdomain) ? (uint64_t(1) << k) : uint64_t(0);
        }
      }
    }
    else {
      (void)a_subdomain;
    }

    return mask;
  };

  // The section plane's labels from the active domain (RegionSettings::m_sectionLabels): each region's subdomain, or
  // excluded.
  const auto sectionLabels = [&]() {
    if constexpr (hasRegionVisibility<S>) {
      const bool any = names.getNumDomains() > 0U;

      for (uint32_t k = 0; k < 64U; k++) {
        const uint32_t d = any ? names.getDomain(names.getActiveDomain()).subdomainOf(k) : Subdomains::None;

        settings.m_regions.m_sectionLabels[k] = (d < 32U) ? uint8_t(d) : RegionSettings::ExcludedLabel;
      }
    }
  };

  // How many of the object's pieces (those found) some selectors hold, all or part of, in words.
  const auto piecesOfAll = [&](const std::vector<SurfaceSelector>& a_selectors) {
    DomainSetup    alone;
    const uint32_t group = alone.addSurfaceGroup();

    for (std::size_t i = 0; i < a_selectors.size(); i++) {
      alone.add(group, a_selectors[i]);
    }

    uint32_t count = 0;

    for (std::size_t c = 0; c < possible.size(); c++) {
      count += (foundOf[c] != std::size_t(-1) && coverageOfGroups(alone, c).m_coverage != Coverage::None) ? 1U : 0U;
    }

    return std::to_string(count) + ((count == 1U) ? " piece" : " pieces");
  };

  const auto piecesOf = [&](const SurfaceSelector& a_selector) {
    return piecesOfAll(std::vector<SurfaceSelector>{a_selector});
  };

  // Whether a surface group's selector holds pieces that are all in another surface group (an earlier one holds them
  // first), so that it gives this one nothing.
  const auto shadowed = [&](const SurfaceSelector& a_selector, const uint32_t a_group) {
    DomainSetup alone;

    alone.add(alone.addSurfaceGroup(), a_selector);

    const std::vector<PieceCoverage>& covered = coverages();

    bool matched = false;

    for (std::size_t c = 0; c < possible.size(); c++) {
      if (foundOf[c] != std::size_t(-1) && coverageOfGroups(alone, c).m_coverage != Coverage::None) {
        matched = true;

        // Held here, only partly held, or held by several surface groups (perhaps this one too): not shadowed.
        if (covered[c].m_surfaceGroup == a_group || covered[c].m_surfaceGroup == SurfaceGroups::None ||
            covered[c].m_coverage != Coverage::Full) {
          return false;
        }
      }
    }

    return matched;
  };

  // Highlight a surface group's surfaces (RegionSettings::Highlighted), or none (None): the picked surfaces' highlight
  // comes back. One highlight at a time: a surface group's surfaces, or some regions' (a subdomain's), or the picked
  // surfaces.
  const auto highlight = [&](const uint32_t a_group, const uint64_t a_regions) {
    if constexpr (hasRegionVisibility<S>) {
      settings.m_regions.m_highlightGroup   = a_group;
      settings.m_regions.m_highlightRegions = a_regions;

      showHighlight();
    }
    else {
      (void)a_group;
      (void)a_regions;
    }
  };

  const auto highlightGroup = [&](const uint32_t a_group) { highlight(a_group, 0U); };

  // What the panel's buttons act on: what is picked, or the piece stepped to with Shift and the wheel (none when
  // nothing is).
  const auto pickedNow = [&]() {
    std::vector<Picked> entries = selection;

    if constexpr (hasRegionVisibility<S>) {
      if (entries.empty() && picked) {
        const SurfaceRef<T> piece = shape.pieceAt(pick.m_point, pick.m_normal);

        if (piece.m_surface != SurfaceRef<T>::None) {
          entries.push_back(Picked{piece, selectorOf(piece)});
        }
      }
    }

    return entries;
  };

  // Add what is picked (or the whole pieces, their surfaces, their regions' surfaces, the interfaces of their two
  // regions) to the current surface group, or remove what holds the pieces there; false when nothing is picked.
  const auto editPicked = [&](const PanelAction a_action) {
    if constexpr (hasRegionVisibility<S>) {
      DomainSetup& setup = edited;

      const std::vector<Picked> entries = pickedNow();

      if (entries.empty()) {
        return false;
      }

      for (const Picked& entry : entries) {
        if (a_action == PanelAction::RemovePiece) {
          // Every selector of the current surface group that holds the piece goes.
          for (uint32_t i = setup.getNumSelectors(); i > 0U; i--) {
            const SurfaceSelector selector = setup.getSelector(i - 1U);

            if (setup.getSurfaceGroupOfSelector(i - 1U) == currentGroup && selector.matches(entry.m_piece)) {
              setup.remove(currentGroup, selector);
            }
          }
        }
        else {
          const SurfaceSelector selector =
            (a_action == PanelAction::AddPicked) ? entry.m_selector : selectorFor(a_action, entry.m_piece);

          if (setup.getNumSurfaceGroups() == 0U) {
            currentGroup = setup.addSurfaceGroup();
          }

          // The same selector is held once (several pieces of one surface add the surface once).
          setup.remove(currentGroup, selector);

          setup.add(currentGroup, selector);
        }
      }

      rekeyPick();

      return true;
    }
    else {
      (void)a_action;

      return false;
    }
  };

  // Pick what the pick mode picks of the piece at a hit; with a_together, add it to what is picked, or take it out if
  // it is there. False when there is no piece there.
  const auto pickAt = [&](const HitT<T>& a_hit, const bool a_together) {
    if constexpr (hasRegionVisibility<S>) {
      const SurfaceRef<T> piece = a_hit.hit() ? shape.pieceAt(a_hit.m_point, a_hit.m_normal) : SurfaceRef<T>();

      if (piece.m_surface == SurfaceRef<T>::None) {
        return false;
      }

      // The piece stepped to before joins first.
      selection = a_together ? pickedNow() : std::vector<Picked>();

      const Picked entry{piece, selectorOf(piece)};

      const auto same = std::find_if(selection.begin(), selection.end(), [&](const Picked& a_entry) {
        return sameSelector(a_entry.m_selector, entry.m_selector);
      });

      if (same != selection.end()) {
        selection.erase(same);
      }
      else if (selection.size() < RegionSettings::MaxSelected) {
        selection.push_back(entry);
      }
      else {
        std::printf("EBGeometry viewer: at most %u surfaces are picked together\n",
                    unsigned(RegionSettings::MaxSelected));
        std::fflush(stdout);
      }

      pick   = a_hit;
      picked = !selection.empty();

      rekeyPick();

      return true;
    }
    else {
      (void)a_hit;
      (void)a_together;

      return false;
    }
  };

  // Pick by another mode: what is picked is picked again from the same pieces (once each).
  const auto setPickMode = [&](const PickMode a_mode) {
    pickMode = a_mode;

    std::vector<Picked> repicked;

    for (const Picked& entry : selection) {
      const Picked again{entry.m_piece, selectorOf(entry.m_piece)};

      if (std::none_of(repicked.begin(), repicked.end(), [&](const Picked& a_entry) {
            return sameSelector(a_entry.m_selector, again.m_selector);
          })) {
        repicked.push_back(again);
      }
    }

    selection = repicked;

    showHighlight();
  };

  // Delete a surface group, keeping the current one where it was.
  const auto deleteGroup = [&](const uint32_t a_group) {
    if constexpr (hasRegionVisibility<S>) {
      DomainSetup& setup = edited;

      if (a_group < setup.getNumSurfaceGroups()) {
        setup.removeSurfaceGroup(a_group);
        names.removeSurfaceGroup(a_group);

        if (currentGroup > a_group || currentGroup >= setup.getNumSurfaceGroups()) {
          currentGroup = (currentGroup > 0U) ? currentGroup - 1U : 0U;
        }

        // The hidden surface groups after it move down by one.
        const uint32_t hidden = settings.m_regions.m_hiddenGroups;
        const uint32_t below  = (1U << a_group) - 1U;

        settings.m_regions.m_hiddenGroups = (hidden & below) | ((hidden >> (a_group + 1U)) << a_group);

        const uint32_t through = settings.m_regions.m_throughGroups;

        settings.m_regions.m_throughGroups = (through & below) | ((through >> (a_group + 1U)) << a_group);

        // The highlighted surface group moves down with the others, or goes with the one deleted.
        const uint32_t highlighted = settings.m_regions.m_highlightGroup;

        if (highlighted != RegionSettings::None && highlighted >= a_group) {
          highlightGroup((highlighted == a_group) ? RegionSettings::None : highlighted - 1U);
        }

        rekeyPick();
      }
    }
    else {
      (void)a_group;
    }
  };

  // Stepping through the surfaces under the cursor (Shift + wheel): the crossings there, front to back, the one
  // picked, and the primitives in front of it, seen through while stepping (a click ends it).
  std::vector<HitT<T>>  depthHits;
  std::size_t           depthIndex = 0;
  std::vector<uint32_t> depthHidden;

  // Stop stepping through the surfaces under the cursor, keeping what it peeled: the surface stepped to is picked (with
  // a_together, added to what is picked), and those in front of it stay seen through (a view of regions: translucent,
  // as with T; otherwise left out). Shift + T makes them opaque again.
  const auto endStepping = [&](const bool a_together) {
    if (depthHits.empty()) {
      return;
    }

    (void)pickAt(pick, a_together);

    for (const uint32_t id : depthHidden) {
      if constexpr (hasRegionVisibility<S>) {
        settings.m_regions.setSeenThrough(id, true);
      }
      else {
        for (uint32_t i = 0; i < Settings<T>::HiddenPrimitives; i++) {
          if (settings.m_hidden[i] == id) {
            break;
          }

          if (settings.m_hidden[i] == Settings<T>::NoPrimitive) {
            settings.m_hidden[i] = id;

            break;
          }
        }
      }
    }

    depthHits.clear();
    depthHidden.clear();
  };

  // Measuring: none, one point, or two (Shift and a click marks them).
  int      measured = 0;
  Vec3T<T> measureA = Vec3T<T>::zeros();
  Vec3T<T> measureB = Vec3T<T>::zeros();

  // Whether a region is seen through: by itself (its box in the panel), or by its key under the coloring now (T), the
  // region's or its material's. Under another coloring, a key seen through belongs to no one region.
  const auto regionSeenThrough = [&](const uint32_t a_region) {
    if constexpr (hasRegionVisibility<S>) {
      const RegionSettings& regions = settings.m_regions;

      if (regions.isRegionSeenThrough(a_region)) {
        return true;
      }

      uint32_t key = RegionSettings::None;

      if (regions.m_coloring == RegionColoring::Region) {
        key = a_region;
      }
      else if (regions.m_coloring == RegionColoring::Material) {
        const MaterialLabelId material = shape.getTape().getMaterialOf(a_region);

        key = material.isNone() ? RegionSettings::None : material.value();
      }

      return key != RegionSettings::None && regions.isSeenThrough(key);
    }
    else {
      (void)a_region;

      return false;
    }
  };

  // The probe: a point a right click marks (where the ray meets a surface, or the section plane in front of it), and
  // where the right button went down (a drag moves the light instead).
  bool     probed     = false;
  Vec3T<T> probePoint = Vec3T<T>::zeros();
  int      rightX     = 0;
  int      rightY     = 0;
  bool     rightMoved = false;

  // The point under the cursor on the section plane, and the shape's value there.
  bool     cursorOn    = false;
  Vec3T<T> cursorPoint = Vec3T<T>::zeros();
  T        cursorValue = T(0);

  // Lines of equal distance on the section plane: off (0), as the settings give them (1, only when they set a
  // spacing), every 1/20 of the box's diagonal (2), every 1/50 (3), every 1/100 (4).
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
  bool overlayOn = false;

  // Whether the overlay and the panel show details: numbers, coordinates, statistics (D).
  bool details = false;

  // The menu the overlay shows (see keys()): one at a time, so that the overlay stays short.
  std::size_t           menu          = 0;
  SDL_Texture*          overlay       = nullptr;
  int                   overlayWidth  = 0;
  int                   overlayHeight = 0;
  std::vector<uint32_t> overlayPixels;

  // The axis planes through the origin, drawn over the image to check where the model sits: none (0), all three (1),
  // or the one facing the camera most (2) (U).
  int axisPlanes = 0;

  // The axis triad, in the bottom left corner, always drawn.
  SDL_Texture*          axes     = nullptr;
  int                   axesSize = 0;
  std::vector<uint32_t> axesPixels;

  // The colorbar of a cost view, in the bottom right corner.
  SDL_Texture*          colorbar       = nullptr;
  int                   colorbarWidth  = 0;
  int                   colorbarHeight = 0;
  std::vector<uint32_t> colorbarPixels;

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

    // The panel of a view of regions takes the top left corner, the overlay of menus the top right; without a panel,
    // the overlay is at the top left.
    const bool withPanel = panel != nullptr && !panelLines.empty();

    if (overlay != nullptr) {
      int outputWidth  = 0;
      int outputHeight = 0;

      SDL_GetRendererOutputSize(screen, &outputWidth, &outputHeight);

      const SDL_Rect where{withPanel ? outputWidth - overlayWidth - 8 : 8, 8, overlayWidth, overlayHeight};

      SDL_RenderCopy(screen, overlay, nullptr, &where);
    }

    if (withPanel) {
      const SDL_Rect where{8, 8, panelWidth, panelHeight};

      SDL_RenderCopy(screen, panel, nullptr, &where);
    }

    if (axes != nullptr) {
      int outputWidth  = 0;
      int outputHeight = 0;

      SDL_GetRendererOutputSize(screen, &outputWidth, &outputHeight);

      const SDL_Rect where{8, outputHeight - axesSize - 8, axesSize, axesSize};

      SDL_RenderCopy(screen, axes, nullptr, &where);
    }

    if (colorbar != nullptr) {
      int outputWidth  = 0;
      int outputHeight = 0;

      SDL_GetRendererOutputSize(screen, &outputWidth, &outputHeight);

      // Left out of a window too narrow for it beside the axes.
      if (colorbarWidth + axesSize + 24 <= outputWidth) {
        const SDL_Rect where{
          outputWidth - colorbarWidth - 8, outputHeight - colorbarHeight - 8, colorbarWidth, colorbarHeight};

        SDL_RenderCopy(screen, colorbar, nullptr, &where);
      }
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

    // The axis planes, over the image: each plane through the origin (xy, yz, zx) a grid over the model's box (stretched
    // to hold the origin), in the color of its normal's axis (x red, y green, z blue), the axes in it drawn stronger.
    // Lines are drawn in short pieces, so that what lies behind a perspective camera is left out.
    if (axisPlanes > 0) {
      const auto bounds = box;

      Vec3T<T> lo = bounds.getLowCorner();
      Vec3T<T> hi = bounds.getHighCorner();

      bool bounded = true;

      for (int k = 0; k < 3; k++) {
        bounded = bounded && std::isfinite(lo[k]) && std::isfinite(hi[k]) && lo[k] <= hi[k];
      }

      for (int k = 0; k < 3; k++) {
        const T reach = bounded ? T(0) : camera.m_distance;

        lo[k] = bounded ? Math::min(lo[k], T(0)) : -reach;
        hi[k] = bounded ? Math::max(hi[k], T(0)) : reach;

        const T margin = T(0.1) * (hi[k] - lo[k]);

        lo[k] -= margin;
        hi[k] += margin;
      }

      Vec3T<T> forward;
      Vec3T<T> right;
      Vec3T<T> up;

      camera.frame(forward, right, up);

      // The plane facing the camera most: the one whose normal is most along the view.
      int facing = 0;

      for (int k = 1; k < 3; k++) {
        facing = (std::abs(forward[k]) > std::abs(forward[facing])) ? k : facing;
      }

      const uint8_t colors[3][3] = {{230, 60, 60}, {60, 190, 60}, {70, 110, 240}};

      const auto segment = [&](const Vec3T<T>& a_from, const Vec3T<T>& a_to) {
        constexpr int pieces = 24;

        for (int s = 0; s < pieces; s++) {
          int x0 = 0;
          int y0 = 0;
          int x1 = 0;
          int y1 = 0;

          const Vec3T<T> p0 = a_from + (T(s) / T(pieces)) * (a_to - a_from);
          const Vec3T<T> p1 = a_from + (T(s + 1) / T(pieces)) * (a_to - a_from);

          if (project(p0, x0, y0) && project(p1, x1, y1)) {
            SDL_RenderDrawLine(screen, x0, y0, x1, y1);
          }
        }
      };

      SDL_SetRenderDrawBlendMode(screen, SDL_BLENDMODE_BLEND);

      for (int normal = 0; normal < 3; normal++) {
        if (axisPlanes == 2 && normal != facing) {
          continue;
        }

        // The plane's two directions.
        const int u = (normal + 1) % 3;
        const int v = (normal + 2) % 3;

        const auto at = [&](const T a_u, const T a_v) {
          Vec3T<T> point = Vec3T<T>::zeros();

          point[u] = a_u;
          point[v] = a_v;

          return point;
        };

        constexpr int lines = 10;

        for (int i = 0; i <= lines; i++) {
          const bool edge  = (i == 0 || i == lines);
          const T    alpha = edge ? T(170) : T(70);

          SDL_SetRenderDrawColor(
            screen, colors[normal][0], colors[normal][1], colors[normal][2], static_cast<uint8_t>(alpha));

          const T su = lo[u] + (hi[u] - lo[u]) * T(i) / T(lines);
          const T sv = lo[v] + (hi[v] - lo[v]) * T(i) / T(lines);

          segment(at(su, lo[v]), at(su, hi[v]));
          segment(at(lo[u], sv), at(hi[u], sv));
        }

        // The plane's two axes through the origin, stronger.
        SDL_SetRenderDrawColor(screen, colors[normal][0], colors[normal][1], colors[normal][2], 230);

        segment(at(T(0), lo[v]), at(T(0), hi[v]));
        segment(at(lo[u], T(0)), at(hi[u], T(0)));
      }

      SDL_SetRenderDrawBlendMode(screen, SDL_BLENDMODE_NONE);
      SDL_SetRenderDrawColor(screen, 0, 0, 0, 255);
    }

    // The probe, over the image: a cross with a dark rim.
    int px = 0;
    int py = 0;

    if (probed && project(probePoint, px, py)) {
      for (int pass = 0; pass < 2; pass++) {
        const int w = (pass == 0) ? 2 : 1;
        const int r = (pass == 0) ? 8 : 7;

        if (pass == 0) {
          SDL_SetRenderDrawColor(screen, 20, 20, 20, 255);
        }
        else {
          SDL_SetRenderDrawColor(screen, 40, 220, 255, 255);
        }

        for (int d = -w + 1; d < w; d++) {
          SDL_RenderDrawLine(screen, px - r, py + d, px + r, py + d);
          SDL_RenderDrawLine(screen, px + d, py - r, px + d, py + r);
        }
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

  // Show the surface groups as edited: built in a small pool of its own (and its mirror, for a render
  // on a device), when they changed since last shown.
  const auto publishGroups = [&]() {
    if (publishedOnce && edited == published) {
      return;
    }

    auto                host  = std::make_unique<Pool>(hostMemoryResource());
    const SurfaceGroups setup = edited.buildSurfaceGroups(*host);

    host->freeze();

    settings.m_regions.m_surfaceGroups = setup;

    if (GPU::enabled && a_deviceShape != nullptr && GPU::deviceAvailable()) {
      auto device = std::make_unique<Pool>(Pool::mirror(*host, GPU::memoryResource()));

      deviceGroups = setup.rebasedView(*device);

      groupPools.push_back(std::move(device));
    }

    groupPools.push_back(std::move(host));

    published     = edited;
    publishedOnce = true;
  };

  // The settings of a frame at a refinement stage: 0 the preview, 1 full resolution without shadows, 2
  // the full settings.
  const auto stageSettings = [&](const int a_stage) {
    publishGroups();

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

    // While stepping through the surfaces under the cursor, those in front of the one picked are seen through: for a
    // view of regions drawn translucent, as its keys seen through (T) are; otherwise left out, in the entries of
    // m_hidden left free.
    if constexpr (hasRegionVisibility<S>) {
      for (const uint32_t id : depthHidden) {
        frame.m_regions.setSeenThrough(id, true);
      }

      // The surface stepped to is shown as picked, and so solid even where it is seen through.
      if (!depthHits.empty()) {
        const SurfaceRef<T> piece = shape.pieceAt(pick.m_point, pick.m_normal);

        if (piece.m_surface != SurfaceRef<T>::None) {
          frame.m_regions.m_numSelected = 1U;
          frame.m_regions.m_selected[0] = selectorOf(piece);
          frame.m_highlightPrimitive    = RegionSettings::Selected;
          frame.m_highlightCount        = 1U;
        }
      }
    }
    else {
      std::size_t next = 0;

      for (uint32_t i = 0; i < Settings<T>::HiddenPrimitives && next < depthHidden.size(); i++) {
        if (frame.m_hidden[i] == Settings<T>::NoPrimitive) {
          frame.m_hidden[i] = depthHidden[next++];
        }
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
  const auto render = [](const S&             a_host,
                         const S&             a_device,
                         const SurfaceGroups& a_deviceScheme,
                         const Settings<T>&   a_frame,
                         const Camera<T>      a_view,
                         const bool           a_onDevice,
                         Image&               a_image,
                         Background*          a_job) {
    // On a device, the surface groups are read from their pool's mirror.
    Settings<T> frame = a_frame;

    if (a_onDevice) {
      frame.m_regions.m_surfaceGroups = a_deviceScheme;
    }

    const Renderer<T> renderer(frame);

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

  // The slice view's camera turned square to the plane, from its removed side; its target, distance and field of view
  // kept (a pan or a zoom).
  const auto aimAtSlice = [&]() {
    const Settings<T> frame = stageSettings(2);
    const Vec3T<T>    n     = RenderDetail::unit(frame.m_sectionNormal);

    Vec3T<T> first;
    Vec3T<T> second;

    camera.horizontal(first, second);

    camera.m_orthographic = true;
    camera.m_pitch = std::asin(Math::clamp(dot(n, RenderDetail::unit(camera.m_up)), T(-1), T(1))) * T(180) / pi<T>;
    camera.m_yaw   = std::atan2(dot(n, first), dot(n, second)) * T(180) / pi<T>;
  };

  // Leave the slice view: the camera and the settings as they were before it.
  const auto leaveSlice = [&]() {
    if (slicing) {
      camera               = beforeSlice;
      settings.m_floor     = sliceFloor;
      settings.m_shadows   = sliceShadows;
      settings.m_occlusion = sliceOcclusion;
      slicing              = false;
    }
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
        // A view of regions gives the section's field there: a subdomain's function, when the section shows one.
        if constexpr (hasRegionVisibility<S>) {
          const auto shown = RenderDetail::shownAs(shape, settings);

          cursorValue = (settings.m_regions.m_sectionRegions != 0U || settings.m_regions.m_sectionDomain)
                          ? shown.contourValue(p)
                          : shown.signedDistance(p);
        }
        else {
          cursorValue = RenderDetail::shownAs(shape, settings).signedDistance(p);
        }
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
                           : (settings.m_colorBy == ColorBy::Primitive) ? "Colored by object"
                                                                        : "Colored in one color";

    const bool costView = settings.m_viewMode == ViewMode::CostHit || settings.m_viewMode == ViewMode::CostCrossing;

    // A cost view: its scale's ends in the frame's unit, and its rays per second (the colorbar shows the scale).
    const std::string view =
      std::string(viewNames[static_cast<int>(settings.m_viewMode)]) +
      ((costView && shownStats.m_costUnit != CostUnit::None)
         ? ", " + costText(shownStats.m_costLow, shownStats.m_costUnit) + " to " +
             costText(shownStats.m_costHigh, shownStats.m_costUnit) + " (dark to bright)" +
             (shownStats.m_costUnit == CostUnit::Cycles ? ", elapsed on the GPU, waits included" : "") + ", " +
             rateText(shownStats.m_raysPerSecond)
         : std::string());

    // The primitives seen through.
    std::string hidden;

    for (uint32_t i = 0; i < Settings<T>::HiddenPrimitives; i++) {
      if (settings.m_hidden[i] != Settings<T>::NoPrimitive) {
        hidden += (hidden.empty() ? "" : ", ") + std::to_string(settings.m_hidden[i]);
      }
    }

    const std::string object = primitivesAreFaces<S> ? "face" : "object";

    // Each menu's state, in the order of keys(): what a reader needs, and with details (D) the numbers behind it.
    struct MenuState
    {
      std::vector<std::string> m_now;
      std::vector<std::string> m_details;
    };

    const std::string pickedWhat =
      !picked ? std::string("Nothing picked")
              : (primitivesAreFaces<S> ? "Picked face " : "Picked object ") + std::to_string(pick.m_primitive);

    std::vector<MenuState> states{
      {{"Yaw " + fixed(double(camera.m_yaw), 1) + ", pitch " + fixed(double(camera.m_pitch), 1) + ", distance " +
          fixed(double(camera.m_distance), 2),
        std::string(camera.m_orthographic ? "Orthographic" : "Perspective") + ", field of view " +
          fixed(double(camera.m_fieldOfView), 0)},
       {"Target " + point(camera.m_target, 3) + ", near clip " + fixed(double(camera.m_nearClip), 3),
        std::string("Light ") + (settings.m_lightFromCamera ? "from the camera, " : "in the scene, ") +
          point(settings.m_light, 2),
        "Highlights of strength " + fixed(double(settings.m_specular), 2) + ", tightness " +
          fixed(double(settings.m_shininess), 0)}},
      {{view + ", on the " + (onDevice ? "GPU" : "CPU"),
        std::string("Shadows ") + onOff(settings.m_shadows) + ", ambient occlusion " + onOff(settings.m_occlusion) +
          ", floor " + onOff(settings.m_floor) + ", sky " + onOff(settings.m_sky)},
       {std::string(coloring),
        std::to_string(settings.m_supersampling * settings.m_supersampling) + " rays per pixel (" +
          std::to_string(settings.m_supersampling) + " x " + std::to_string(settings.m_supersampling) + ")"}},
      {{std::string("Cut: ") + sectionNames[sectionMode] +
          ((sectionMode != 0) ? std::string(sectionFlip ? ", reversed" : "") + ", moved " +
                                  fixed(double(sectionOffset), 3) + (slicing ? ", slice view" : "")
                              : std::string()),
        (settings.m_isoSpacing > T(0)) ? "Lines of equal distance every " + fixed(double(settings.m_isoSpacing), 4)
                                       : std::string("No lines of equal distance")},
       {"Contour " + fixed(double(settings.m_contourWidth), 1) + " pixels wide, sheet " +
          fixed(100.0 * double(settings.m_sheetOpacity), 0) + "% opaque, cut face " +
          fixed(100.0 * double(settings.m_cutOpacity), 0) + "% opaque",
        cursorOn ? "Under the cursor " + point(cursorPoint, 3) + ", distance " + fixed(double(cursorValue), 4)
                 : std::string("Nothing under the cursor on the plane")}},
      {{pickedWhat,
        hidden.empty() ? std::string("Every surface opaque") : "Seen through: " + object + " " + hidden,
        (measured == 2)   ? "Measured " + fixed(double((measureB - measureA).length()), 4)
        : (measured == 1) ? std::string("Measuring: Shift + click the second point")
                          : std::string("Nothing measured"),
        probed ? "Probe at " + point(probePoint, 3) + ", distance " +
                   fixed(double(RenderDetail::shownAs(shape, settings).signedDistance(probePoint)), 4)
               : std::string("No probe: right click a point")},
       {picked ? "Picked at " + point(pick.m_point, 3) + ", normal " + point(pick.m_normal, 2)
               : std::string("No picked point"),
        depthHits.empty() ? std::string("Not stepping through surfaces")
                          : "Surface " + std::to_string(depthIndex + 1) + " of " + std::to_string(depthHits.size()) +
                              " under the cursor",
        (measured == 2) ? "Along x, y, z " + point(measureB - measureA, 4) + ", from " + point(measureA, 3)
                        : std::string("No measurement")}},
      {{}, {}}};

    // The regions' menu and the panel, for a shape whose regions can be shown (see RegionView).
    panelLines.clear();
    panelSwatches.clear();
    panelActions.clear();

    if constexpr (hasRegionVisibility<S>) {
      const RegionSettings& regions = settings.m_regions;
      const DomainSetup&    setup   = edited;

      // Names, with their numbers only with details.
      const auto region  = [&](const uint32_t a_region) { return names.region(a_region, details); };
      const auto surface = [&](const uint32_t a_surface) { return names.surface(a_surface, details); };
      const auto group   = [&](const uint32_t a_group) { return names.surfaceGroup(a_group, details); };

      // A piece in words: its surface (and part, with details) between its owner and its neighbor.
      const auto pieceWords = [&](const SurfaceRef<T>& a_piece) {
        return surface(a_piece.m_surface) + (details ? ", part " + std::to_string(a_piece.m_part) : std::string()) +
               ", between " + region(a_piece.m_ownerRegion) + " and " + region(a_piece.m_neighborRegion);
      };

      // A selector in words: what surfaces, then where.
      const auto selectorWords = [&](const SurfaceSelector& a_selector) {
        constexpr uint32_t Any = SurfaceSelector::Any;

        if (a_selector.m_rest) {
          return std::string("All remaining surfaces (those no other entry holds)");
        }

        const bool hasSurface = a_selector.m_surface != Any;
        const bool hasPart    = a_selector.m_part != Any;
        const bool hasRegions = a_selector.m_regionA != Any;
        const bool hasLabel   = a_selector.m_label != Any;
        const bool hasMater   = a_selector.m_materialA != Any;

        std::string words = hasSurface ? surface(a_selector.m_surface) +
                                           (hasPart ? ", part " + std::to_string(a_selector.m_part) : std::string())
                            : hasLabel ? "Surfaces labeled " + names.surfaceLabel(a_selector.m_label, details)
                                       : std::string("All surfaces");

        if (hasSurface && !hasRegions && !hasMater) {
          return "All of " + words;
        }

        if (hasRegions) {
          words += (a_selector.m_regionB != Any)
                     ? ", between " + region(a_selector.m_regionA) + " and " + region(a_selector.m_regionB)
                     : " of " + region(a_selector.m_regionA);
        }

        if (hasMater) {
          words += (a_selector.m_materialB != Any)
                     ? ", between materials " + names.material(a_selector.m_materialA, details) + " and " +
                         names.material(a_selector.m_materialB, details)
                     : " of material " + names.material(a_selector.m_materialA, details);
        }

        return words;
      };

      const auto keyName = [&](const uint32_t a_key) {
        if (a_key == RegionSettings::None) {
          return std::string("nothing");
        }

        switch (regions.m_coloring) {
        case RegionColoring::Region: {
          return region(a_key);
        }
        case RegionColoring::Material: {
          return names.material(a_key, details);
        }
        case RegionColoring::SurfaceLabel: {
          return names.surfaceLabel(a_key, details);
        }
        case RegionColoring::Surface: {
          return surface(a_key);
        }
        case RegionColoring::Interface: {
          return "between " + region(a_key >> 16U) + " and " + region(a_key & 0xFFFFU);
        }
        case RegionColoring::SurfaceGroup: {
          return group(a_key);
        }
        default: {
          return "part " + std::to_string(a_key);
        }
        }
      };

      // The regions' menu shows what the panel does not: what is shown alone or seen through.
      std::string through;

      for (uint32_t i = 0; i < RegionSettings::MaxSeenThrough; i++) {
        if (regions.m_through[i] != RegionSettings::None) {
          through += (through.empty() ? "" : "; ") + keyName(regions.m_through[i]);
        }
      }

      states.insert(states.end() - 1,
                    MenuState{{(regions.m_only != RegionSettings::None) ? "Showing only " + keyName(regions.m_only)
                                                                        : std::string("Showing everything visible"),
                               through.empty() ? std::string("Nothing seen through") : "Seen through: " + through},
                              {}});

      // What the section plane shows: the whole domain's functions, a subdomain's, or the visible regions'.
      if (regions.m_sectionDomain && names.getNumDomains() > 0U) {
        states[2].m_now.push_back("Section shows: every subdomain's function of " +
                                  names.domainName(names.getActiveDomain()));
      }
      else if (regions.m_sectionRegions != 0U) {
        std::string field = "Section shows: the function of a set of regions";

        for (uint32_t d = 0;
             names.getNumDomains() > 0U && d < names.getDomain(names.getActiveDomain()).getNumSubdomains();
             d++) {
          if (subdomainAllRegions(d) == regions.m_sectionRegions) {
            field = "Section shows: the function of subdomain " + names.subdomain(d, details) + " of " +
                    names.domainName(names.getActiveDomain());
          }
        }

        states[2].m_now.push_back(field);
      }
      else {
        states[2].m_now.push_back("Section shows: the visible regions' function");
      }

      // In the status, the picked piece in words.
      states[3].m_now.front() = (selection.size() > 1U) ? "Picked: " + std::to_string(selection.size()) + " together"
                                : picked ? "Picked: " + pieceWords(shape.pieceAt(pick.m_point, pick.m_normal))
                                         : std::string("Nothing picked");

      // The panel: lines, buttons (clicked) and swatches (a key's color).
      const auto line = [&](const std::string& a_text) {
        panelLines.push_back(a_text);
        panelActions.emplace_back();
        panelSwatches.push_back(0U);
      };

      const auto swatch = [&](const uint32_t a_key) { panelSwatches.back() = swatchOf(a_key); };

      const auto button = [&](const std::string& a_label, const PanelAction a_action, const uint32_t a_value) {
        std::string& text = panelLines.back();

        text += (text.empty() || text.back() == ' ') ? "" : " ";

        const std::size_t begin = text.size();

        text += "[" + a_label + "]";

        panelActions.back().push_back(PanelSpan{begin, text.size(), a_action, a_value});
      };

      // A whole line that acts when clicked anywhere on it.
      const auto clickable = [&](const std::string& a_text, const PanelAction a_action, const uint32_t a_value) {
        line(a_text);

        panelActions.back().push_back(PanelSpan{0, std::size_t(-1), a_action, a_value});
      };

      line("");
      button(details ? "Hide details" : "Show details", PanelAction::Details, 0U);

      line("");
      // The domains, one a line; under the active one, its subdomains and the regions each computes in. A button
      // reads what it will do; one that is on is starred.
      if (names.getNumDomains() > 0U) {
        const DomainSetup& domain = names.getDomain(names.getActiveDomain());
        const uint64_t     lit    = regions.m_highlightRegions;

        // Whether every drawn region of a set is hidden (false for none).
        const auto allHidden = [&](const uint64_t a_mask) {
          bool hidden = a_mask != 0U;

          for (uint32_t k = 1; k < 64U; k++) {
            if (((a_mask >> k) & 1U) != 0U) {
              hidden = hidden && !regions.isVisible(k, shape.getNumRegions());
            }
          }

          return hidden;
        };

        line("Domains (click one to see it highlighted, and its subdomains)");

        for (uint32_t i = 0; i < names.getNumDomains(); i++) {
          if (i != names.getActiveDomain()) {
            line("    ");
            button(names.domainName(i), PanelAction::SelectDomain, i);

            continue;
          }

          // The active domain, marked, and starred while highlighted.
          const uint64_t drawn = subdomainRegions(Subdomains::None);

          line("  > ");
          button(
            std::string((drawn != 0U && lit == drawn) ? "* " : "") + names.domainName(i), PanelAction::SelectDomain, i);
          button(allHidden(drawn) ? "Show" : "Hide", PanelAction::HideDomain, 0U);
          button("Only", PanelAction::OnlyDomain, 0U);

          // Its subdomains, each with its regions.
          for (uint32_t d = 0; d < domain.getNumSubdomains(); d++) {
            std::string in;

            for (uint32_t k = 0; k < shape.getNumRegions(); k++) {
              if (domain.subdomainOf(k) == d) {
                in += (in.empty() ? "" : ", ") + ((k == 0U) ? region(k) + " (outside everything)" : region(k));
              }
            }

            // Its drawn regions (the background is never drawn: nothing of it to hide or show alone), and all of
            // them, the background's included, whose surfaces a highlight lights.
            const uint64_t mask = subdomainRegions(d);
            const uint64_t all  = subdomainAllRegions(d);

            // Its name highlights it (starred while it is), then its regions.
            line("      ");
            button(std::string((all != 0U && lit == all) ? "* " : "") + "Subdomain " + names.subdomain(d, details),
                   PanelAction::HighlightSubdomain,
                   d);

            if (mask != 0U) {
              button(allHidden(mask) ? "Show" : "Hide", PanelAction::HideSubdomain, d);
              button("Only", PanelAction::OnlySubdomain, d);
            }

            line("          " + in);
          }
        }

        line("  ");
        button("Show all regions", PanelAction::ShowAllRegions, 0U);
        line("");
      }

      line("Color by");
      line("  ");

      for (uint32_t c = 0; c < std::size(colorChoices); c++) {
        const ColorChoice& choice  = colorChoices[c];
        const bool         current = settings.m_colorBy == choice.m_colorBy &&
                             (choice.m_colorBy != ColorBy::Primitive || regions.m_coloring == choice.m_coloring);

        if (c == 4U) {
          line("  ");
        }

        button(std::string(current ? "* " : "") + choice.m_label, PanelAction::ColorBy, c);
      }

      line("");
      // What a click picks, what is picked, and the ways to add it to the current surface group.
      const std::vector<Picked> chosen  = pickedNow();
      const bool                byPart  = pickMode == PickMode::Surface;
      const bool                several = chosen.size() > 1U;

      // The probed point: where it is, the region it is in, the active domain's label and functions there, and the
      // nearest surface of its region with its surface group.
      if (probed) {
        const auto&    tape = shape.getTape();
        const uint32_t in   = tape.region(probePoint);

        line("Probe (right click; on the background, clears it)");
        line("  At " + point(probePoint, 4) + ", in " + region(in));

        if (names.getNumDomains() > 0U) {
          // The active domain built for the readout alone, in a pool of its own.
          Pool           tables(hostMemoryResource());
          const Domain   domain(tables, tape, names.getDomain(names.getActiveDomain()));
          const uint32_t label = domain.labelAt(probePoint);
          const uint32_t count = domain.getNumLabels();

          std::vector<T> values(count);

          domain.signedDistances(probePoint, values.data(), count);

          line("  " + names.domainName(names.getActiveDomain()) + ": " +
               ((label == Subdomains::None) ? std::string("excluded")
                                            : "in subdomain " + names.subdomain(label, details)));

          for (uint32_t l = 0; l < count; l++) {
            line("    " + ((l + 1U == count) ? std::string("Excluded") : "Subdomain " + names.subdomain(l, details)) +
                 ": function " + fixed(double(values[l]), 4));
          }
        }

        const SurfaceRef<T> nearest = tape.closestSurface(probePoint, in);

        if (nearest.m_surface != SurfaceRef<T>::None) {
          const uint32_t holds = setup.surfaceGroupOf(nearest);

          line("  Nearest surface " + pieceWords(nearest));
          line("    " + ((holds == SurfaceGroups::None) ? std::string("In no surface group") : "In " + group(holds)) +
               ", region's function " + fixed(double(tape.signedDistance(probePoint, in)), 4));
        }

        line("");
      }

      line("Pick");
      line("  ");
      button(std::string(byPart ? "* " : "") + "Surface (the part clicked)", PanelAction::PickMode, 0U);
      button(std::string(byPart ? "" : "* ") + "Region", PanelAction::PickMode, 1U);
      line("  Ctrl + click picks several");

      if (chosen.empty()) {
        line("  Nothing picked: click a surface");
      }
      else {
        line(several ? "  Picked (" + std::to_string(chosen.size()) + ")" : std::string("  Picked"));

        for (std::size_t i = 0; i < chosen.size(); i++) {
          const SurfaceRef<T>& piece = chosen[i].m_piece;

          if (byPart) {
            const uint32_t in = setup.surfaceGroupOf(piece);

            line("    " + surface(piece.m_surface) + ", part " + std::to_string(piece.m_part) + ", between " +
                 region(piece.m_ownerRegion) + " and " + region(piece.m_neighborRegion) +
                 ((in == SurfaceGroups::None) ? std::string(", in no surface group") : ", in " + group(in)));
          }
          else {
            line("    " + region(chosen[i].m_selector.m_regionA) + ", all its surfaces");
          }

          if (several) {
            button("x", PanelAction::Unpick, uint32_t(i));
          }
        }

        if (!several && details) {
          line("    At " + point(pick.m_point, 3) + ", normal " + point(pick.m_normal, 2));
        }

        if (!selection.empty()) {
          line("    ");
          button("Pick nothing", PanelAction::UnpickAll, 0U);
        }

        // The ways to add them, each saying what it adds and how much of the object that is.
        line("  Add to " +
             ((setup.getNumSurfaceGroups() > 0U) ? group(currentGroup) : std::string("a new surface group")));

        const SurfaceRef<T>& first = chosen.front().m_piece;

        std::vector<std::pair<PanelAction, std::string>> ways;

        if (byPart) {
          ways.emplace_back(PanelAction::AddPicked, several ? "These parts" : "This part");
          ways.emplace_back(PanelAction::AddPiece,
                            several ? "Their whole pieces, all parts" : "The whole piece, all parts");
          ways.emplace_back(PanelAction::AddSurface,
                            several ? std::string("All of their surfaces") : "All of " + surface(first.m_surface));
          ways.emplace_back(PanelAction::AddRegion,
                            several ? std::string("All surfaces of their regions")
                                    : "All surfaces of " + region(first.m_ownerRegion));
        }
        else {
          ways.emplace_back(PanelAction::AddPicked,
                            several ? std::string("All surfaces of these regions") : std::string("All its surfaces"));
        }

        ways.emplace_back(PanelAction::AddInterface,
                          several ? std::string("Everything between their two regions")
                                  : "Everything between " + region(first.m_ownerRegion) + " and " +
                                      region(first.m_neighborRegion));

        for (const auto& [action, words] : ways) {
          std::vector<SurfaceSelector> selectors;

          for (const Picked& entry : chosen) {
            selectors.push_back((action == PanelAction::AddPicked) ? entry.m_selector
                                                                   : selectorFor(action, entry.m_piece));
          }

          line("    ");
          button(words, action, 0U);
          panelLines.back() += "  " + piecesOfAll(selectors);
        }

        if (setup.getNumSurfaceGroups() > 0U) {
          line("  ");
          button("Remove from " + group(currentGroup), PanelAction::RemovePiece, 0U);
        }
      }

      line("");
      line("Regions (box: shown, ~ translucent, off; name: highlight)");

      // The regions' states are 64-bit masks (RegionSettings): from region 64 on, a region is always shown, opaque.
      if (shape.getNumRegions() > 64U) {
        line("  Regions from 64 on (" + std::to_string(shape.getNumRegions() - 64U) +
             " here) are always shown and opaque: their boxes do nothing");
      }

      // What else hides or fades surfaces, beside the boxes below: the keys seen through (T, or stepping), the key shown
      // alone (L), the surface groups made translucent or hidden, and the camera inside a region; with a way to undo
      // all but the camera (the regions' own boxes are kept).
      {
        std::vector<std::string> others;
        std::string              through;

        for (uint32_t i = 0; i < RegionSettings::MaxSeenThrough; i++) {
          if (regions.m_through[i] != RegionSettings::None) {
            through += (through.empty() ? "" : ", ") + keyName(regions.m_through[i]);
          }
        }

        if (!through.empty()) {
          others.push_back("seen through: " + through + " (T)");
        }

        if (regions.m_only != RegionSettings::None) {
          others.push_back("showing only " + keyName(regions.m_only) + " (L)");
        }

        for (uint32_t g = 0; g < setup.getNumSurfaceGroups(); g++) {
          if (regions.isSurfaceGroupHidden(g)) {
            others.push_back("surface group " + group(g) + " hidden");
          }
          else if (regions.isSurfaceGroupSeenThrough(g)) {
            others.push_back("surface group " + group(g) + " translucent");
          }
        }

        const bool inside = shownStats.m_cameraInside;

        if (inside) {
          others.push_back("the camera is inside " + region(shape.getTape().region(camera.eye())) + " (zoom out)");
        }

        if (!others.empty()) {
          line("  Also hiding or fading surfaces:");

          for (const std::string& other : others) {
            line("    " + other);
          }

          if (!through.empty() || regions.m_only != RegionSettings::None || regions.m_hiddenGroups != 0U ||
              regions.m_throughGroups != 0U) {
            line("    ");
            button("Make all opaque", PanelAction::MakeOpaque, 0U);
          }
        }
      }

      // The background, everything outside: never drawn, but named, with its material's color.
      line("      " + region(0) + ", outside everything" +
           (regions.isRegionHighlighted(0) && regions.m_highlightRegions == 1U ? " (highlighted)" : ""));

      panelActions.back().push_back(PanelSpan{6, panelLines.back().size(), PanelAction::HighlightRegion, 0U});

      if (settings.m_colorBy == ColorBy::Primitive && regions.m_coloring == RegionColoring::Material) {
        swatch(names.materialOf(0));
      }

      for (uint32_t k = 1; k < shape.getNumRegions(); k++) {
        const bool inDomain = names.getNumDomains() > 0U && names.getDomain(names.getActiveDomain()).computesIn(k);

        // The box shows or hides the region; its name highlights its surfaces.
        const bool shown   = regions.isVisible(k, shape.getNumRegions());
        const bool through = shown && regionSeenThrough(k);

        line(std::string(!shown    ? "  [ ] "
                         : through ? "  [~] "
                                   : "  [x] ") +
             region(k) + (inDomain ? ", in the domain" : "") + (through ? ", translucent" : "") +
             ((k < 64U && regions.m_highlightRegions == (uint64_t(1) << k)) ? " (highlighted)" : ""));

        panelActions.back().push_back(PanelSpan{2, 5, PanelAction::ToggleRegion, k});
        panelActions.back().push_back(PanelSpan{6, panelLines.back().size(), PanelAction::HighlightRegion, k});

        if (settings.m_colorBy == ColorBy::Primitive && regions.m_coloring == RegionColoring::Region) {
          swatch(k);
        }
        else if (settings.m_colorBy == ColorBy::Primitive && regions.m_coloring == RegionColoring::Material) {
          swatch(names.materialOf(k));
        }
      }

      line("");
      line("Surface groups (click to select and highlight)");

      for (uint32_t g = 0; g < setup.getNumSurfaceGroups(); g++) {
        if (g == renaming) {
          line("  > " + renameText + "_");
          line("      Type its name; Enter keeps it, Esc cancels");

          continue;
        }

        line(std::string(g == currentGroup ? "  > " : "    "));

        panelLines.back() += group(g) + ((regions.m_highlightGroup == g) ? " (highlighted)" : "") +
                             (regions.isSurfaceGroupHidden(g)        ? " (hidden)"
                              : regions.isSurfaceGroupSeenThrough(g) ? " (translucent)"
                                                                     : "");
        panelActions.back().push_back(PanelSpan{0, panelLines.back().size(), PanelAction::CurrentGroup, g});

        if (settings.m_colorBy == ColorBy::Primitive && regions.m_coloring == RegionColoring::SurfaceGroup) {
          swatch(g);
        }

        button(regions.isSurfaceGroupHidden(g)        ? "Show"
               : regions.isSurfaceGroupSeenThrough(g) ? "Hide"
                                                      : "Translucent",
               PanelAction::HideGroup,
               g);
        button("Rename", PanelAction::RenameGroup, g);

        button("Copy", PanelAction::CopyGroup, g);

        button("Delete", PanelAction::DeleteGroup, g);

        for (uint32_t i = 0; i < setup.getNumSelectors(); i++) {
          if (setup.getSurfaceGroupOfSelector(i) == g) {
            line("        " + selectorWords(setup.getSelector(i)) + ": " + piecesOf(setup.getSelector(i)) +
                 (shadowed(setup.getSelector(i), g) ? " (all held by another surface group first)" : ""));
            button("x", PanelAction::RemoveSelector, i);
          }
        }
      }

      line("  ");

      button("+ New surface group", PanelAction::NewGroup, 0U);

      if (setup.getNumSurfaceGroups() > 0U) {
        button("Add remaining surfaces to " + group(currentGroup), PanelAction::AddRest, 0U);
      }

      // The pieces the object may have: those found, those possible but not found, and how the surface groups cover them.
      const std::vector<PieceCoverage>& covered = coverages();

      uint32_t found       = 0;
      uint32_t unassigned  = 0;
      uint32_t partly      = 0;
      bool     findableGap = false;

      uint32_t touched = 0;

      for (std::size_t c = 0; c < possible.size(); c++) {
        if (!relevant(c)) {
          continue;
        }

        touched++;
        found += (foundOf[c] != std::size_t(-1)) ? 1U : 0U;
        unassigned += (covered[c].m_coverage == Coverage::None) ? 1U : 0U;
        partly += (covered[c].m_coverage == Coverage::Partly) ? 1U : 0U;
        findableGap = findableGap || (covered[c].m_coverage != Coverage::Full && foundOf[c] != std::size_t(-1));
      }

      const uint32_t notFound = touched - found;

      line("");
      line((names.getNumDomains() > 0U) ? "Pieces of surface " + names.domainName(names.getActiveDomain()) + " touches"
                                        : std::string("Pieces of surface"));
      line("  " + std::to_string(found) + " found" +
           ((notFound > 0U) ? ", " + std::to_string(notFound) + " more possible but not found" : std::string()));
      line((unassigned + partly == 0U) ? std::string("  All in a surface group")
                                       : "  " + std::to_string(unassigned) + " in no surface group" +
                                           ((partly > 0U) ? ", " + std::to_string(partly) + " partly" : std::string()));

      if (details) {
        line("  Found by 64 x 64 rays along each axis; possible ones from the expression");
      }

      line("  ");

      if (findableGap) {
        button("Next unassigned", PanelAction::NextUnassigned, 0U);
      }

      button(listPieces ? "Hide list" : "List", PanelAction::ListPieces, 0U);

      if (listPieces) {
        for (std::size_t c = 0, listedPieces = 0; c < possible.size() && listedPieces < 40U; c++) {
          if (!relevant(c)) {
            continue;
          }

          listedPieces++;

          SurfaceRef<T> s;

          s.m_surface        = possible[c].m_surface;
          s.m_ownerRegion    = possible[c].m_ownerRegion;
          s.m_neighborRegion = possible[c].m_neighborRegion;

          const PieceCoverage& in   = covered[c];
          const bool           seen = foundOf[c] != std::size_t(-1);
          std::string          words =
            surface(s.m_surface) + ", between " + region(s.m_ownerRegion) + " and " + region(s.m_neighborRegion) + ": ";

          words += (in.m_coverage == Coverage::None)            ? std::string("none")
                   : (in.m_coverage == Coverage::Partly)        ? std::string("partly")
                   : (in.m_surfaceGroup != SurfaceGroups::None) ? group(in.m_surfaceGroup)
                                                                : std::string("several surface groups");

          if (!seen) {
            words += " (possible, not found)";
          }
          else if (details) {
            std::string parts;

            for (const uint32_t part : pieces[foundOf[c]].m_parts) {
              parts += (parts.empty() ? "" : ", ") + std::to_string(part);
            }

            words += " (parts " + parts + "; crossed " + std::to_string(pieces[foundOf[c]].m_crossings) + " times)";
          }

          if (seen) {
            clickable("    " + words, PanelAction::PickPiece, static_cast<uint32_t>(c));
          }
          else {
            line("    " + words);
          }
        }

        if (touched > 40U) {
          line("    And " + std::to_string(touched - 40U) + " more");
        }
      }
    }

    const std::vector<KeyGroup> groups = keys(a_deviceShape != nullptr, hasRegionVisibility<S>);

    // The status: the frame, what is picked (and the frame's statistics, with details), then the row of menus, the
    // one shown in brackets. Every key works whichever menu is shown.
    std::vector<std::string> lines{std::string(shownWhat) + ", " + std::to_string(shownWidth) + " x " +
                                   std::to_string(shownHeight) + " pixels in " + fixed(1000.0 * shownSeconds, 0) +
                                   " ms on the " + (onDevice ? "GPU" : "CPU")};

    if (details) {
      lines.emplace_back("Rays " + std::to_string(shownStats.m_rays) + ", hits " +
                         percent(shownStats.m_hits, shownStats.m_rays) + ", seen from inside " +
                         std::to_string(shownStats.m_insideHits) + ", unresolved " +
                         std::to_string(shownStats.m_unresolved));
    }

    lines.emplace_back(states[3].m_now.front());

    // From inside a region (the air around an object, zoomed into), its outer faces cannot show as a block.
    if (shownStats.m_cameraInside) {
      std::string where = "the solid";

      if constexpr (hasRegionVisibility<S>) {
        where = names.region(shape.getTape().region(camera.eye()));
      }

      lines.emplace_back("The camera is inside " + where + ": zoom out to see it from outside");
    }

    std::string tabs;

    for (std::size_t g = 0; g < groups.size(); g++) {
      const bool shown = overlayOn && g == menu;

      tabs += (g > 0 ? "  " : "") + std::string(shown ? "[" : "") + "F" + std::to_string(g + 1) + " " +
              groups[g].m_short + (shown ? "]" : "");
    }

    lines.emplace_back(tabs);
    lines.emplace_back(overlayOn ? "Tab: next menu, H: hide, D: details" : "H: show the menu, D: details");

    // The menu shown: its state, then its keys under their headings.
    if (overlayOn && menu < groups.size()) {
      lines.emplace_back("");
      lines.emplace_back(groups[menu].m_title);
      lines.emplace_back("  Now");

      for (const std::string& state : states[menu].m_now) {
        lines.emplace_back("    " + state);
      }

      if (details) {
        for (const std::string& state : states[menu].m_details) {
          lines.emplace_back("    " + state);
        }
      }

      if (groups[menu].m_keys.empty() || groups[menu].m_keys.front().m_keys != nullptr) {
        lines.emplace_back("  Keys");
      }

      for (const Key& key : groups[menu].m_keys) {
        if (key.m_keys == nullptr) {
          lines.emplace_back(std::string("  ") + key.m_what);

          continue;
        }

        char text[160];

        std::snprintf(text, sizeof(text), "    %-20s %s", key.m_keys, key.m_what);
        lines.emplace_back(text);
      }
    }

    // Every line leads with a capital letter, past its indentation and markers.
    for (std::vector<std::string>* text : {&lines, &panelLines}) {
      for (std::string& l : *text) {
        l = sentenceCase(l);
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

      // The swatches: a square in the first two columns of their lines.
      for (std::size_t row = 0; row < panelSwatches.size(); row++) {
        if (panelSwatches[row] == 0U) {
          continue;
        }

        const int top  = 6 * scale + static_cast<int>(row) * GlyphHeight * scale;
        const int side = Math::min(2 * GlyphWidth, GlyphHeight) * scale - 2 * scale;

        for (int y = top + scale; y < top + scale + side; y++) {
          for (int x = 6 * scale + scale; x < 6 * scale + scale + side; x++) {
            panelPixels[std::size_t(y) * std::size_t(panelWidth) + std::size_t(x)] = panelSwatches[row];
          }
        }
      }

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

    // The colorbar of the frame shown, in a cost view.
    if (costView && shownStats.m_costUnit != CostUnit::None) {
      const int oldWidth  = colorbarWidth;
      const int oldHeight = colorbarHeight;

      drawColorbar(shownStats, onDevice, scale, colorbarPixels, colorbarWidth, colorbarHeight);

      if (colorbar == nullptr || oldWidth != colorbarWidth || oldHeight != colorbarHeight) {
        if (colorbar != nullptr) {
          SDL_DestroyTexture(colorbar);
        }

        colorbar = SDL_CreateTexture(
          screen, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, colorbarWidth, colorbarHeight);

        SDL_SetTextureBlendMode(colorbar, SDL_BLENDMODE_BLEND);
      }

      SDL_UpdateTexture(colorbar, nullptr, colorbarPixels.data(), 4 * colorbarWidth);
    }
    else if (colorbar != nullptr) {
      SDL_DestroyTexture(colorbar);

      colorbar = nullptr;
    }
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

    // A pick with no key (a surface in no group, colored by group) has nothing to outline: the background has no key
    // either.
    // What is highlighted or picked is outlined: in a view of regions, the surfaces of a highlighted subdomain,
    // domain or surface group, or each surface picked, along their own edges; otherwise the one picked, around what
    // shows of it.
    constexpr uint32_t Outside = 0xFFFFFFFFU;

    std::vector<SurfaceSelector> outlinedBy;

    if constexpr (hasRegionVisibility<S>) {
      const RegionSettings& regions = settings.m_regions;

      if (regions.m_highlightRegions != 0U) {
        // Every surface with a highlighted region on either side.
        for (uint32_t k = 0; k < 64U && outlinedBy.size() < RegionSettings::MaxSelected; k++) {
          if (regions.isRegionHighlighted(k)) {
            outlinedBy.push_back(SurfaceSelector::region(k));
          }
        }
      }
      else if (regions.m_highlightGroup != RegionSettings::None) {
        // The highlighted surface group's selectors (the remaining surfaces, which no selector names, are not).
        for (uint32_t i = 0; i < edited.getNumSelectors() && outlinedBy.size() < RegionSettings::MaxSelected; i++) {
          const SurfaceSelector selector = edited.getSelector(i);

          if (edited.getSurfaceGroupOfSelector(i) == regions.m_highlightGroup && !selector.m_rest) {
            outlinedBy.push_back(selector);
          }
        }
      }
      else if (settings.m_highlightPrimitive == RegionSettings::Selected) {
        for (const Picked& entry : selection) {
          outlinedBy.push_back(entry.m_selector);
        }
      }
    }

    const bool     several = !outlinedBy.empty();
    const uint32_t key     = pick.m_primitive;
    const bool     outline = several || (picked && (primitivesAreFaces<S> || key != Outside));

    // The surfaces outlined, rendered again alone at the frame's size, each side of each keyed apart: their edges are
    // where the keys change, at their silhouettes and at their rims, wherever the rest of the object hides them.
    Image edges;

    Settings<T> shown = stageSettings(1);

    shown.m_width    = a_image.m_width;
    shown.m_height   = a_image.m_height;
    shown.m_viewMode = ViewMode::Shaded;

    Settings<T> alone = shown;

    alone.m_recordPrimitives       = true;
    alone.m_regions.m_onlySelected = true;
    alone.m_regions.m_numSelected  = static_cast<uint32_t>(outlinedBy.size());

    for (std::size_t i = 0; i < outlinedBy.size(); i++) {
      alone.m_regions.m_selected[i] = outlinedBy[i];
    }

    if constexpr (hasRegionVisibility<S>) {
      if (outline && several) {
        (void)render(shape, deviceShape, deviceGroups, alone, camera, onDevice, edges, nullptr);
      }
    }

    const Image& keyed = several ? edges : a_image;

    if (outline && keyed.m_primitives.size() == a_image.m_pixels.size()) {
      // A pixel's outlined object: Outside, or what it is (all of a mesh SDF is one; a picked surface's side).
      const auto objectAt = [&](const int a_x, const int a_y) {
        const uint32_t id = keyed.m_primitives[std::size_t(a_y) * a_image.m_width + std::size_t(a_x)];

        if (primitivesAreFaces<S>) {
          return (id != Outside) ? 0U : Outside;
        }

        if (several) {
          return (id - RegionSettings::Selected < 2U * RegionSettings::MaxSelected) ? id : Outside;
        }

        return (id == key) ? id : Outside;
      };

      const int width  = static_cast<int>(a_image.m_width);
      const int height = static_cast<int>(a_image.m_height);
      const int reach  = 2;

      outlined = a_image.m_pixels;

      // An edge of the picked surfaces is hidden where the object is hit clearly in front of it. The edge lies between
      // the outline's pixel and the one beside it that showed it, at the nearer of the picked surfaces there (the
      // other may be a surface's back, seen past its rim); the ray through that pixel meets the object as shown
      // before it. Hidden edges are not drawn.
      const Renderer<T> object(shown);
      const Renderer<T> picks(alone);

      const auto hidden = [&](const int a_x, const int a_y, const int a_u, const int a_v) {
        const HitT<T> here   = picks.pick(shape, camera, T(a_x) + T(0.5), T(a_y) + T(0.5));
        const HitT<T> beside = picks.pick(shape, camera, T(a_u) + T(0.5), T(a_v) + T(0.5));
        const bool    nearer = here.hit() && (!beside.hit() || here.m_t < beside.m_t);
        const HitT<T> edge   = nearer ? here : beside;

        const HitT<T> front = nearer ? object.pick(shape, camera, T(a_x) + T(0.5), T(a_y) + T(0.5))
                                     : object.pick(shape, camera, T(a_u) + T(0.5), T(a_v) + T(0.5));

        // Clearly: by more than a few pixels' width at the edge's depth (at the camera's distance, orthographic).
        const T across    = T(2) * std::tan(T(0.5) * camera.m_fieldOfView * pi<T> / T(180)) / T(height);
        const T footprint = across * (camera.m_orthographic ? camera.m_distance : edge.m_t);

        return edge.hit() && front.hit() && front.m_t < edge.m_t - T(4) * footprint;
      };

      // A pixel is on the outline when an outlined object other than its own is near: outside them all, or on the
      // lower-keyed side of a border between two (a line one reach wide either way).
      for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
          const uint32_t own = objectAt(x, y);

          bool near  = false;
          int  nearU = x;
          int  nearV = y;

          for (int dy = -reach; dy <= reach && !near; dy++) {
            for (int dx = -reach; dx <= reach && !near; dx++) {
              const int u = x + dx;
              const int v = y + dy;

              if (u >= 0 && v >= 0 && u < width && v < height) {
                const uint32_t other = objectAt(u, v);

                near  = other != Outside && other != own && (own == Outside || own < other);
                nearU = u;
                nearV = v;
              }
            }
          }

          if (near) {
            const std::size_t pixel = std::size_t(y) * a_image.m_width + std::size_t(x);

            // Where the object hides the edge, nothing is drawn.
            if (!several || !hidden(x, y, nearU, nearV)) {
              outlined[pixel] = 0xFFB81AFFU;
            }
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
          // In the slice view a drag pans: an orbit would turn the plane away.
          panning  = slicing || (SDL_GetModState() & KMOD_CTRL) != 0;
          dragging = !panning;
          pressX   = event.button.x;
          pressY   = event.button.y;
          moved    = false;
        }
        else if (event.button.button == SDL_BUTTON_RIGHT) {
          lighting   = true;
          rightX     = event.button.x;
          rightY     = event.button.y;
          rightMoved = false;
        }

        break;
      }
      case SDL_MOUSEBUTTONUP: {
        // A click anywhere keeps the name being typed.
        if (renaming != SurfaceGroups::None) {
          finishRename(true);
        }

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

              const int left   = 8;
              const int row    = (y - 8 - 6 * panelScale) / (GlyphHeight * panelScale);
              const int column = (x - left - 6 * panelScale) / (GlyphWidth * panelScale);

              // Only a button or a clickable line is the panel's: a click elsewhere on it (it is translucent) picks
              // the surface behind it.
              PanelSpan clicked;

              if (!panelLines.empty() && x >= left + 6 * panelScale && x < left + panelWidth &&
                  y >= 8 + 6 * panelScale && y < 8 + panelHeight && row >= 0 &&
                  std::size_t(row) < panelActions.size()) {
                for (const PanelSpan& span : panelActions[std::size_t(row)]) {
                  if (std::size_t(column) >= span.m_begin && std::size_t(column) < span.m_end) {
                    clicked = span;
                  }
                }
              }

              if (clicked.m_action != PanelAction::None) {
                RegionSettings& regions = settings.m_regions;
                DomainSetup&    setup   = edited;

                switch (clicked.m_action) {
                case PanelAction::AddPicked:
                case PanelAction::AddPiece:
                case PanelAction::AddSurface:
                case PanelAction::AddInterface:
                case PanelAction::AddRegion:
                case PanelAction::RemovePiece: {
                  (void)editPicked(clicked.m_action);

                  break;
                }
                case PanelAction::Unpick: {
                  if (clicked.m_value < selection.size()) {
                    selection.erase(selection.begin() + std::ptrdiff_t(clicked.m_value));
                  }

                  picked = picked && !selection.empty();

                  rekeyPick();

                  break;
                }
                case PanelAction::MakeOpaque: {
                  // Every key opaque and shown, every surface group shown; the regions keep their boxes.
                  for (uint32_t i = 0; i < RegionSettings::MaxSeenThrough; i++) {
                    regions.m_through[i] = RegionSettings::None;
                  }

                  regions.m_only          = RegionSettings::None;
                  regions.m_throughGroups = 0U;
                  regions.m_hiddenGroups  = 0U;

                  rekeyPick();

                  break;
                }
                case PanelAction::UnpickAll: {
                  picked = false;

                  clearSelection();

                  break;
                }
                case PanelAction::PickMode: {
                  setPickMode((clicked.m_value == 1U) ? PickMode::Region : PickMode::Surface);

                  break;
                }
                case PanelAction::ToggleRegion: {
                  // The box cycles: shown, translucent (see Settings::m_seenThroughOpacity), off, shown again. A
                  // region seen through by its key (T) is translucent too; turned off, its key is made opaque.
                  const uint32_t k = clicked.m_value;

                  if (!regions.isVisible(k, shape.getNumRegions())) {
                    regions.setVisible(k, true);
                  }
                  else if (!regionSeenThrough(k)) {
                    regions.setRegionSeenThrough(k, true);
                  }
                  else {
                    const MaterialLabelId material = shape.getTape().getMaterialOf(k);

                    regions.setRegionSeenThrough(k, false);

                    if (regions.m_coloring == RegionColoring::Region) {
                      regions.setSeenThrough(k, false);
                    }
                    else if (regions.m_coloring == RegionColoring::Material && !material.isNone()) {
                      regions.setSeenThrough(material.value(), false);
                    }

                    regions.setVisible(k, false);
                  }

                  rekeyPick();

                  break;
                }
                case PanelAction::CurrentGroup: {
                  // A click makes a surface group current and highlights its surfaces; again, it stops.
                  currentGroup = clicked.m_value;

                  highlightGroup((regions.m_highlightGroup == clicked.m_value) ? RegionSettings::None
                                                                               : clicked.m_value);

                  break;
                }
                case PanelAction::DeleteGroup: {
                  deleteGroup(clicked.m_value);

                  break;
                }
                case PanelAction::RemoveSelector: {
                  setup.remove(setup.getSurfaceGroupOfSelector(clicked.m_value), setup.getSelector(clicked.m_value));

                  rekeyPick();

                  break;
                }
                case PanelAction::NewGroup: {
                  currentGroup = setup.addSurfaceGroup();

                  // A new surface group is named first.
                  startRename(currentGroup);

                  break;
                }
                case PanelAction::RenameGroup: {
                  currentGroup = clicked.m_value;

                  startRename(clicked.m_value);

                  break;
                }
                case PanelAction::NextUnassigned:
                case PanelAction::PickPiece: {
                  // The next found piece not wholly in a surface group after the last one picked from the pieces, or
                  // the one clicked.
                  std::size_t index = possible.size();

                  if (clicked.m_action == PanelAction::PickPiece) {
                    index = clicked.m_value;
                  }
                  else {
                    const std::vector<PieceCoverage>& covered = coverages();

                    for (std::size_t step = 1; step <= possible.size(); step++) {
                      const std::size_t i = (pieceIndex + step) % possible.size();

                      if (relevant(i) && covered[i].m_coverage != Coverage::Full && foundOf[i] != std::size_t(-1)) {
                        index = i;

                        break;
                      }
                    }
                  }

                  if (index < possible.size() && foundOf[index] != std::size_t(-1)) {
                    const SurfacePiece<T>& piece = pieces[foundOf[index]];

                    pieceIndex = index;

                    HitT<T> at;

                    at.m_t          = T(0);
                    at.m_point      = piece.m_point;
                    at.m_normal     = piece.m_normal;
                    camera.m_target = piece.m_point;

                    if (!pickAt(at, false)) {
                      pick.m_point  = piece.m_point;
                      pick.m_normal = piece.m_normal;
                      picked        = true;

                      selection.clear();

                      rekeyPick();
                    }
                  }

                  break;
                }
                case PanelAction::ColorBy: {
                  const ColorChoice& choice = colorChoices[clicked.m_value];

                  settings.m_colorBy = choice.m_colorBy;

                  if (choice.m_colorBy == ColorBy::Primitive) {
                    regions.setColoring(choice.m_coloring);
                  }

                  applyColors();
                  rekeyPick();

                  break;
                }
                case PanelAction::AddRest: {
                  setup.remove(currentGroup, SurfaceSelector::rest());
                  setup.add(currentGroup, SurfaceSelector::rest());

                  rekeyPick();

                  break;
                }
                case PanelAction::CopyGroup: {
                  // The copy holds the same selectors; a surface is in the first surface group that holds it, so the
                  // copy holds what the original leaves until one of them is changed.
                  const uint32_t copy = setup.addSurfaceGroup();
                  const uint32_t from = clicked.m_value;

                  for (uint32_t i = 0, n = setup.getNumSelectors(); i < n; i++) {
                    if (setup.getSurfaceGroupOfSelector(i) == from) {
                      setup.add(copy, setup.getSelector(i));
                    }
                  }

                  names.setSurfaceGroup(copy, names.surfaceGroup(from, false) + " (copy)");

                  currentGroup = copy;

                  rekeyPick();

                  break;
                }
                case PanelAction::SelectDomain: {
                  // The active domain keeps its surface groups as edited; the new one's are shown.
                  if (clicked.m_value != names.getActiveDomain()) {
                    names.getDomain(names.getActiveDomain()) = setup;

                    names.setActiveDomain(clicked.m_value);

                    setup                    = names.getDomain(clicked.m_value);
                    currentGroup             = 0;
                    regions.m_hiddenGroups   = 0U;
                    regions.m_throughGroups  = 0U;
                    regions.m_highlightGroup = RegionSettings::None;

                    // A section showing a domain or a subdomain shows the new domain.
                    regions.m_sectionDomain  = regions.m_sectionDomain || regions.m_sectionRegions != 0U;
                    regions.m_sectionRegions = 0U;

                    sectionLabels();
                    pieceIndex = std::size_t(-1);

                    applyColors();
                    rekeyPick();

                    regions.m_highlightRegions = 0U;
                  }

                  // The domain clicked is highlighted, its drawn regions' surfaces (a second click stops it).
                  const uint64_t mask = subdomainRegions(Subdomains::None);

                  highlight(RegionSettings::None, (regions.m_highlightRegions == mask) ? uint64_t(0) : mask);

                  break;
                }
                case PanelAction::HighlightSubdomain: {
                  const uint64_t mask = subdomainAllRegions(clicked.m_value);

                  // A section showing a subdomain shows the one clicked.
                  if (regions.m_sectionRegions != 0U) {
                    regions.m_sectionRegions = mask;
                  }

                  highlight(RegionSettings::None, (regions.m_highlightRegions == mask) ? uint64_t(0) : mask);

                  break;
                }
                case PanelAction::HighlightRegion: {
                  const uint64_t mask = (clicked.m_value < 64U) ? (uint64_t(1) << clicked.m_value) : uint64_t(0);

                  highlight(RegionSettings::None, (regions.m_highlightRegions == mask) ? uint64_t(0) : mask);

                  break;
                }
                case PanelAction::HideSubdomain:
                case PanelAction::HideDomain: {
                  // A subdomain shown again if all its regions are hidden, hidden otherwise; a domain hidden.
                  const uint64_t mask = subdomainRegions(
                    (clicked.m_action == PanelAction::HideDomain) ? Subdomains::None : clicked.m_value);

                  bool hiddenAll = clicked.m_action == PanelAction::HideSubdomain;

                  for (uint32_t k = 1; k < 64U; k++) {
                    if (((mask >> k) & 1U) != 0U) {
                      hiddenAll = hiddenAll && !regions.isVisible(k, shape.getNumRegions());
                    }
                  }

                  for (uint32_t k = 1; k < 64U; k++) {
                    if (((mask >> k) & 1U) != 0U) {
                      regions.setVisible(k, hiddenAll);
                    }
                  }

                  rekeyPick();

                  break;
                }
                case PanelAction::OnlySubdomain:
                case PanelAction::OnlyDomain: {
                  const uint64_t mask = subdomainRegions(
                    (clicked.m_action == PanelAction::OnlyDomain) ? Subdomains::None : clicked.m_value);

                  for (uint32_t k = 1; k < 64U; k++) {
                    regions.setVisible(k, ((mask >> k) & 1U) != 0U);
                  }

                  rekeyPick();

                  break;
                }
                case PanelAction::ShowAllRegions: {
                  regions.m_hidden = 0U;

                  rekeyPick();

                  break;
                }
                case PanelAction::HideGroup: {
                  // Shown, translucent, hidden, shown again.
                  const uint32_t g = clicked.m_value;

                  if (regions.isSurfaceGroupHidden(g)) {
                    regions.setSurfaceGroupHidden(g, false);
                  }
                  else if (!regions.isSurfaceGroupSeenThrough(g)) {
                    regions.setSurfaceGroupSeenThrough(g, true);
                  }
                  else {
                    regions.setSurfaceGroupSeenThrough(g, false);
                    regions.setSurfaceGroupHidden(g, true);
                  }

                  rekeyPick();

                  break;
                }
                case PanelAction::Details: {
                  details = !details;

                  break;
                }
                case PanelAction::ListPieces: {
                  listPieces = !listPieces;

                  break;
                }
                default: {
                  break;
                }
                }

                dragging = false;
                panning  = false;
                changed  = true;

                break;
              }
            }

            // A click on the scene picks a surface: no surface group is highlighted then (a measuring click keeps it).
            if constexpr (hasRegionVisibility<S>) {
              if ((SDL_GetModState() & KMOD_SHIFT) == 0) {
                highlightGroup(RegionSettings::None);
              }
            }

            const bool together = hasRegionVisibility<S> && (SDL_GetModState() & KMOD_CTRL) != 0;

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

            // A click while stepping through surfaces picks the one stepped to (with Ctrl, adds it to what is
            // picked), and stops stepping.
            if (!depthHits.empty()) {
              endStepping(together);

              dragging = false;
              panning  = false;
              changed  = true;

              break;
            }

            // In a view of regions, a click on a surface picks what the pick mode picks there: the part clicked, or
            // the region; with Ctrl, adds it to what is picked, or takes it out. A double-click also centers the
            // camera on the point.
            if (pickAt(hit, together)) {
              if (event.button.clicks >= 2) {
                camera.m_target = hit.m_point;
              }

              dragging = false;
              panning  = false;
              changed  = true;

              break;
            }

            pick   = hit;
            picked = pick.hit();

            clearSelection();

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

          // A right click (no drag) probes the point under the cursor: the surface there, or the section plane where
          // it shows in front of it; on the background, it clears the probe.
          if (!rightMoved) {
            int windowWidth  = 0;
            int windowHeight = 0;

            SDL_GetWindowSize(window, &windowWidth, &windowHeight);

            const Settings<T> frame  = stageSettings(2);
            const T           column = T(event.button.x) * T(frame.m_width) / T(Math::max(windowWidth, 1));
            const T           row    = T(event.button.y) * T(frame.m_height) / T(Math::max(windowHeight, 1));
            const HitT<T>     hit    = Renderer<T>(frame).pick(shape, camera, column, row);

            Vec3T<T> forward;
            Vec3T<T> right;
            Vec3T<T> up;

            camera.frame(forward, right, up);

            const auto depth = [&](const Vec3T<T>& a_point) { return dot(a_point - camera.eye(), forward); };

            readCursor(event.button.x, event.button.y);

            const bool onPlane = frame.m_section && cursorOn && (!hit.hit() || depth(cursorPoint) < depth(hit.m_point));

            probed     = onPlane || hit.hit();
            probePoint = onPlane ? cursorPoint : hit.m_point;

            if (probed) {
              std::printf("EBGeometry viewer: probe at (%g, %g, %g)\n",
                          double(probePoint[0]),
                          double(probePoint[1]),
                          double(probePoint[2]));
              std::fflush(stdout);
            }

            changed = true;
          }
        }

        break;
      }
      case SDL_MOUSEMOTION: {
        moved = moved || std::abs(event.motion.x - pressX) > 3 || std::abs(event.motion.y - pressY) > 3;

        if (lighting) {
          rightMoved = rightMoved || std::abs(event.motion.x - rightX) > 3 || std::abs(event.motion.y - rightY) > 3;

          turnLight(T(0.4) * T(event.motion.xrel), -T(0.4) * T(event.motion.yrel));

          changed = true;
        }
        else if (panning) {
          // A drag, not the jitter of a click, ends stepping through the surfaces.
          if (moved) {
            endStepping(false);
          }

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
          if (moved) {
            endStepping(false);
          }

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

            if constexpr (hasRegionVisibility<S>) {
              highlightGroup(RegionSettings::None);
            }

            // Stepping picks one surface at a time: what was picked before is dropped first, so that the surfaces
            // are listed by their own keys.
            if (depthHits.empty()) {
              clearSelection();
            }

            Settings<T> frame = stageSettings(2);

            // The surfaces are listed as the user's settings show them, without those stepped past, the translucent
            // ones included (a view of regions): stepping onto one picks it.
            frame.m_hidden = settings.m_hidden;

            if constexpr (hasRegionVisibility<S>) {
              frame.m_regions.clearSeenThrough();
              frame.m_regions.m_numSelected = 0U;
            }

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
              settings.m_highlightCount     = 1U;

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
          else if (slicing) {
            // In the slice view the wheel moves the plane through the object, in coarse steps.
            sectionOffset += T(0.02) * scale * T(event.wheel.y);
          }
          else {
            camera.m_distance *= (event.wheel.y > 0) ? T(0.9) : T(1) / T(0.9);

            endStepping(false);
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
      case SDL_TEXTINPUT: {
        // Typing a surface group's name: printable characters only, up to 40.
        if (renaming != SurfaceGroups::None) {
          for (const char* c = event.text.text; *c != '\0'; c++) {
            if (*c >= 32 && *c <= 126 && renameText.size() < 40U) {
              renameText += *c;
            }
          }

          drawOverlay();
          present();
        }

        break;
      }
      case SDL_KEYDOWN: {
        // While a surface group's name is typed, keys type it: Enter keeps it, Esc cancels, Backspace erases.
        if (renaming != SurfaceGroups::None) {
          const SDL_Keycode key = event.key.keysym.sym;

          if (key == SDLK_RETURN || key == SDLK_KP_ENTER || key == SDLK_ESCAPE) {
            finishRename(key != SDLK_ESCAPE);
          }
          else if (key == SDLK_BACKSPACE && !renameText.empty()) {
            renameText.pop_back();
          }

          drawOverlay();
          present();

          break;
        }

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
        case SDLK_u: {
          // Drawn over the image: nothing to render again.
          axisPlanes = (axisPlanes + 1) % 3;
          changed    = pending;

          present();

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
        case SDLK_z: {
          // The slice view, or back to the view before it. Without a plane, it cuts across x first.
          if (slicing) {
            leaveSlice();

            break;
          }

          if (sectionMode == 0) {
            if constexpr (hasRegionVisibility<S>) {
              RegionSettings& regions = settings.m_regions;

              if (names.getNumDomains() > 0U && !regions.m_sectionDomain && regions.m_sectionRegions == 0U) {
                regions.m_sectionDomain = true;

                sectionLabels();
              }
            }

            sectionMode   = 2;
            sectionOffset = T(0);
          }

          beforeSlice    = camera;
          sliceFloor     = settings.m_floor;
          sliceShadows   = settings.m_shadows;
          sliceOcclusion = settings.m_occlusion;

          settings.m_floor     = false;
          settings.m_shadows   = false;
          settings.m_occlusion = false;
          camera.m_target      = center;
          slicing              = true;

          aimAtSlice();

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
          // diagonal, every 1/50, every 1/100.
          isoMode = (isoMode + 1) % 5;
          isoMode = (isoMode == 1 && givenIso <= T(0)) ? 2 : isoMode;

          settings.m_isoSpacing = (isoMode == 0)   ? T(0)
                                  : (isoMode == 1) ? givenIso
                                                   : scale / ((isoMode == 2)   ? T(20)
                                                              : (isoMode == 3) ? T(50)
                                                                               : T(100));

          break;
        }
        case SDLK_w: {
          // The full frame at the window's size, rendered now, as a PPM image in the working directory.
          Image             image;
          const Settings<T> frame = stageSettings(2);

          (void)render(shape, deviceShape, deviceGroups, frame, camera, onDevice, image, nullptr);

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
        case SDLK_d: {
          details = !details;
          changed = pending;

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

          // The setup built by clicking, to paste into the program.
          if constexpr (hasRegionVisibility<S>) {
            const DomainSetup& setup = edited;

            if (setup.getNumSurfaceGroups() > 0U) {
              if (names.getNumDomains() > 0U) {
                std::printf("// The surface groups of the domain \"%s\"\n",
                            names.domainName(names.getActiveDomain()).c_str());
              }

              std::printf("DomainSetup setup;\n");

              // The subdomains first, so that the setup builds the same domain (Domain(pool, tape, setup)).
              for (uint32_t d = 0; d < setup.getNumSubdomains(); d++) {
                std::string regions;

                for (const uint32_t r : setup.getRegions(d)) {
                  regions += (regions.empty() ? "" : ", ") + std::to_string(r);
                }

                std::printf("(void)setup.addSubdomain({%s}); // %s\n", regions.c_str(), names.subdomain(d).c_str());
              }

              for (uint32_t g = 0; g < setup.getNumSurfaceGroups(); g++) {
                std::printf("const uint32_t group%u = setup.addSurfaceGroup(); // %s\n",
                            unsigned(g),
                            names.surfaceGroup(g).c_str());
              }

              for (uint32_t i = 0; i < setup.getNumSelectors(); i++) {
                std::printf("setup.add(group%u, %s); // %s\n",
                            unsigned(setup.getSurfaceGroupOfSelector(i)),
                            selectorCode(setup.getSelector(i)).c_str(),
                            selectorName(setup.getSelector(i)).c_str());
              }
            }
          }

          std::fflush(stdout);

          changed = pending;

          break;
        }
        case SDLK_x: {
          // X turns the plane on across x, then steps it to y, z and x again; Ctrl and X turn it off; Shift and X
          // make the cut face see-through (the shape's surfaces behind it show), or opaque again.
          if ((event.key.keysym.mod & KMOD_SHIFT) != 0) {
            settings.m_cutOpacity = (settings.m_cutOpacity < T(1)) ? T(1) : T(0.5);

            break;
          }

          if ((event.key.keysym.mod & KMOD_CTRL) != 0) {
            sectionMode = 0;
          }
          else {
            // Turned on with domains registered, the plane shows the active domain first.
            if constexpr (hasRegionVisibility<S>) {
              RegionSettings& regions = settings.m_regions;

              if (sectionMode == 0 && names.getNumDomains() > 0U && !regions.m_sectionDomain &&
                  regions.m_sectionRegions == 0U) {
                regions.m_sectionDomain = true;

                sectionLabels();
              }
            }

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
          // M steps forward through the views, Shift and M back: shaded, x-ray and the two costs. The depth view is left
          // out (a program can still set it); from it, M goes on as from the shaded view.
          constexpr ViewMode cycle[] = {ViewMode::Shaded, ViewMode::XRay, ViewMode::CostHit, ViewMode::CostCrossing};
          constexpr int      count   = 4;

          int at = 0;

          for (int i = 0; i < count; i++) {
            at = (cycle[i] == settings.m_viewMode) ? i : at;
          }

          const int step = ((event.key.keysym.mod & KMOD_SHIFT) != 0) ? count - 1 : 1;

          settings.m_viewMode = cycle[(at + step) % count];

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
          // The panel's buttons for the picked piece, as keys.
          const PanelAction action = (event.key.keysym.sym == SDLK_BACKSPACE)     ? PanelAction::RemovePiece
                                     : ((event.key.keysym.mod & KMOD_CTRL) != 0)  ? PanelAction::AddInterface
                                     : ((event.key.keysym.mod & KMOD_SHIFT) != 0) ? PanelAction::AddSurface
                                                                                  : PanelAction::AddPicked;

          if (!editPicked(action)) {
            changed = pending;
          }

          break;
        }
        case SDLK_DELETE: {
          // Delete the current surface group (and its selectors).
          if constexpr (hasRegionVisibility<S>) {
            deleteGroup(currentGroup);
          }
          else {
            changed = pending;
          }

          break;
        }
        case SDLK_j: {
          if constexpr (hasRegionVisibility<S>) {
            DomainSetup& setup = edited;

            if (currentGroup + 1U < setup.getNumSurfaceGroups()) {
              currentGroup++;
            }
            else {
              currentGroup = setup.addSurfaceGroup();
            }
          }
          else {
            changed = pending;
          }

          break;
        }
        case SDLK_k: {
          if constexpr (hasRegionVisibility<S>) {
            // Group only with surface groups to color by.
            const int count = (edited.getNumSurfaceGroups() > 0U) ? 7 : 6;

            settings.m_regions.setColoring(
              static_cast<RegionColoring>((static_cast<int>(settings.m_regions.m_coloring) + 1) % count));

            settings.m_colorBy = ColorBy::Primitive;

            applyColors();
            rekeyPick();
          }
          else {
            changed = pending;
          }

          break;
        }
        case SDLK_e: {
          // Shift and E draw lines where the domain or subdomain shown does not compute, or stop.
          if ((event.key.keysym.mod & KMOD_SHIFT) != 0) {
            settings.m_isoSecondary = !settings.m_isoSecondary;

            break;
          }

          // The section plane shows, in turn: the whole active domain (each point its own subdomain's function, so
          // that every subdomain's lines show at once), each subdomain's function alone, what a cut-cell code cuts
          // for it, and the visible regions' again (the plane turned on across x if it was off).
          if constexpr (hasRegionVisibility<S>) {
            RegionSettings& regions = settings.m_regions;

            // With no domain registered, only the visible regions' field.
            const bool     any   = names.getNumDomains() > 0U;
            const uint32_t count = any ? names.getDomain(names.getActiveDomain()).getNumSubdomains() : 0U;

            // The step now: 0 the visible regions, 1 the domain, 2 + d subdomain d.
            uint32_t now = regions.m_sectionDomain ? 1U : 0U;

            for (uint32_t d = 0; d < count; d++) {
              if (regions.m_sectionRegions != 0U && subdomainAllRegions(d) == regions.m_sectionRegions) {
                now = 2U + d;
              }
            }

            const uint32_t next = (count == 0U) ? 0U : (now + 1U) % (count + 2U);

            regions.m_sectionDomain  = next == 1U;
            regions.m_sectionRegions = (next >= 2U) ? subdomainAllRegions(next - 2U) : uint64_t(0);

            sectionLabels();

            if ((regions.m_sectionDomain || regions.m_sectionRegions != 0U) && sectionMode == 0) {
              sectionMode   = 2;
              sectionOffset = T(0);
            }
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

                clearSelection();
              }
            }
            else {
              // One region at a time, among those a 64-bit mask can show alone.
              soloRegion = (soloRegion + 1U < Math::min(shape.getNumRegions(), 64U)) ? soloRegion + 1U : 0U;

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
              const bool through = !regions.isSeenThrough(pick.m_primitive);

              regions.setSeenThrough(pick.m_primitive, through);

              // At most RegionSettings::MaxSeenThrough keys: past them, nothing changes, and the pick stays.
              if (through && !regions.isSeenThrough(pick.m_primitive)) {
                std::printf("EBGeometry viewer: at most %u surfaces can be seen through; Shift + T shows them all.\n",
                            unsigned(RegionSettings::MaxSeenThrough));
                std::fflush(stdout);
              }
              else {
                // What is seen through is no longer picked (nor anything picked with it).
                picked = false;

                clearSelection();
              }
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
          // The camera, the light and the picked surfaces, as the window opened with them (out of the slice view).
          leaveSlice();

          camera                        = a_camera;
          settings.m_light              = a_settings.m_light;
          settings.m_hidden             = a_settings.m_hidden;
          settings.m_volumePrimitive    = a_settings.m_volumePrimitive;
          settings.m_highlightPrimitive = a_settings.m_highlightPrimitive;
          settings.m_highlightCount     = a_settings.m_highlightCount;
          picked                        = false;

          selection.clear();
          depthHits.clear();
          depthHidden.clear();

          // A view of regions shows every region again, none alone and none seen through (its coloring and surface groups
          // stay).
          if constexpr (hasRegionVisibility<S>) {
            settings.m_regions.m_hidden = a_settings.m_regions.m_hidden;
            settings.m_regions.m_only   = a_settings.m_regions.m_only;

            settings.m_regions.m_highlightGroup   = RegionSettings::None;
            settings.m_regions.m_highlightRegions = 0U;
            settings.m_regions.m_numSelected      = 0U;
            settings.m_regions.m_sectionRegions   = 0U;
            settings.m_regions.m_sectionDomain    = false;
            settings.m_regions.m_hiddenGroups     = a_settings.m_regions.m_hiddenGroups;
            settings.m_regions.m_throughGroups    = a_settings.m_regions.m_throughGroups;
            settings.m_regions.m_throughRegions   = a_settings.m_regions.m_throughRegions;

            for (uint32_t i = 0; i < RegionSettings::MaxSeenThrough; i++) {
              settings.m_regions.m_through[i] = a_settings.m_regions.m_through[i];
            }

            soloRegion = 0;
          }

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
      // The slice view follows the plane (X, N), and ends with it (Ctrl + X) or with a perspective camera (O).
      if (slicing && (sectionMode == 0 || !camera.m_orthographic)) {
        // O asked for a perspective camera: the one before the slice view, in perspective.
        const bool perspective = sectionMode != 0;

        leaveSlice();

        camera.m_orthographic = perspective ? false : camera.m_orthographic;
      }
      else if (slicing) {
        aimAtSlice();
      }

      if (job != nullptr) {
        job->stop();
        job.reset();
      }

      Image image;

      const auto start = Clock::now();
      // What was under the cursor may have moved with the camera or the plane; read again on the next move.
      cursorOn = false;

      const Stats stats = render(shape, deviceShape, deviceGroups, stageSettings(0), camera, onDevice, image, nullptr);
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

      // The frame's settings first: they publish the surface groups the device view below reads.
      const Settings<T> stage = stageSettings(pending);

      next->m_stage  = pending;
      next->m_height = stage.m_height;
      next->m_thread = std::thread([next, render, shape, deviceShape, deviceGroups, stage, camera, onDevice]() {
        const auto start = Clock::now();

        next->m_stats   = render(shape, deviceShape, deviceGroups, stage, camera, onDevice, next->m_image, next);
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

  if (colorbar != nullptr) {
    SDL_DestroyTexture(colorbar);
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
