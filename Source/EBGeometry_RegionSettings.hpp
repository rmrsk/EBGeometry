// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_RegionSettings.hpp
 * @brief  RegionSettings: how the regions of an object are shown (visibility, coloring, a solver's scheme).
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_REGIONSETTINGS_HPP
#define EBGEOMETRY_REGIONSETTINGS_HPP

// Std includes
#include <cstdint>
#include <type_traits>

// Our includes
#include "EBGeometry_GPU.hpp"
#include "EBGeometry_Scheme.hpp"

namespace EBGeometry {

/**
 * @brief What a view of regions names a surface by, the key a renderer colors by (Render::ColorBy::Primitive).
 */
enum class RegionColoring : uint8_t
{
  Region,       ///< The region the surface belongs to, as seen.
  Material,     ///< That region's material.
  SurfaceLabel, ///< The surface's label (SurfaceLabel()).
  Surface,      ///< The surface itself (SurfaceRef::m_surface): each shape, each member of a collection.
  Part,         ///< The surface's part: a box's face, a mesh's face.
  Interface,    ///< The pair of regions on its two sides there (see RegionSettings::interfaceKey()).
  SurfaceGroup  ///< The surface group of the scheme the surface is in (RegionSettings::m_scheme), or none.
};

/**
 * @brief How the regions of an object are shown (see RegionView): which regions are visible, what a surface is
 * colored and named by, the surfaces shown alone or seen through, and a solver's scheme of groups.
 * @details A plain value, part of the render settings (Render::Settings::m_regions), so that a render, a viewer and
 * a saved view share one description, and a kernel takes it. Keys are by the coloring: a region's number, a
 * material's, a surface label's, a surface's, a part's, an interface's (interfaceKey()) or a group's.
 */
struct RegionSettings
{
  /**
   * @brief No key.
   */
  static constexpr uint32_t None = 0xFFFFFFFFU;

  /**
   * @brief How many keys can be seen through at once.
   */
  static constexpr uint32_t MaxSeenThrough = 8;

  /**
   * @brief The hidden regions, a bit each (bit k for region k); regions from 64 on are always shown.
   */
  uint64_t m_hidden = 0U;

  /**
   * @brief What a surface is colored and named by.
   */
  RegionColoring m_coloring = RegionColoring::Region;

  /**
   * @brief The key shown alone, or None.
   */
  uint32_t m_only = None;

  /**
   * @brief The keys seen through; None marks an unused entry.
   */
  uint32_t m_through[MaxSeenThrough] = {None, None, None, None, None, None, None, None};

  /**
   * @brief A solver's groups of surfaces (RegionColoring::SurfaceGroup colors by them).
   */
  Scheme m_scheme;

  /**
   * @brief The key of the interface between two regions, the same whichever side it is seen from.
   * @param[in] a_first  A region.
   * @param[in] a_second The region on the other side.
   * @return The key.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  static inline uint32_t
  interfaceKey(const uint32_t a_first, const uint32_t a_second) noexcept
  {
    return ((a_first < a_second ? a_first : a_second) << 16U) | (a_first < a_second ? a_second : a_first);
  }

  /**
   * @brief Whether a region is shown (the background never is).
   * @param[in] a_region     Region.
   * @param[in] a_numRegions How many regions the object has.
   * @return True if visible.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline bool
  isVisible(const uint32_t a_region, const uint32_t a_numRegions) const noexcept
  {
    return a_region > 0U && a_region < a_numRegions && (a_region >= 64U || ((m_hidden >> a_region) & 1U) == 0U);
  }

  /**
   * @brief Show or hide a region.
   * @param[in] a_region  Region (1 to 63; others are always shown).
   * @param[in] a_visible Whether to show it.
   */
  EBGEOMETRY_HOST_DEVICE
  inline void
  setVisible(const uint32_t a_region, const bool a_visible) noexcept
  {
    if (a_region > 0U && a_region < 64U) {
      const uint64_t bit = uint64_t(1) << a_region;

      m_hidden = a_visible ? (m_hidden & ~bit) : (m_hidden | bit);
    }
  }

  /**
   * @brief Show one region and hide the others.
   * @param[in] a_region Region.
   */
  EBGEOMETRY_HOST_DEVICE
  inline void
  showOnly(const uint32_t a_region) noexcept
  {
    m_hidden = ~uint64_t(0);

    this->setVisible(a_region, true);
  }

  /**
   * @brief Whether the surfaces with a key are seen through.
   * @param[in] a_key The key.
   * @return True if they are.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline bool
  isSeenThrough(const uint32_t a_key) const noexcept
  {
    for (uint32_t i = 0; i < MaxSeenThrough; i++) {
      if (m_through[i] == a_key && a_key != None) {
        return true;
      }
    }

    return false;
  }

  /**
   * @brief See through the surfaces of a key, or make them opaque again (at most MaxSeenThrough keys; beyond them,
   * none is added).
   * @param[in] a_key     The key.
   * @param[in] a_through Whether to see through it.
   */
  EBGEOMETRY_HOST_DEVICE
  inline void
  setSeenThrough(const uint32_t a_key, const bool a_through) noexcept
  {
    for (uint32_t i = 0; i < MaxSeenThrough; i++) {
      if (m_through[i] == a_key) {
        m_through[i] = None;
      }
    }

    if (a_through) {
      for (uint32_t i = 0; i < MaxSeenThrough; i++) {
        if (m_through[i] == None) {
          m_through[i] = a_key;

          break;
        }
      }
    }
  }

  /**
   * @brief Make every surface opaque.
   */
  EBGEOMETRY_HOST_DEVICE
  inline void
  clearSeenThrough() noexcept
  {
    for (uint32_t i = 0; i < MaxSeenThrough; i++) {
      m_through[i] = None;
    }
  }

  /**
   * @brief Color and name surfaces by another key; the keys shown alone and seen through, which meant another
   * thing, are cleared.
   * @param[in] a_coloring The coloring.
   */
  EBGEOMETRY_HOST_DEVICE
  inline void
  setColoring(const RegionColoring a_coloring) noexcept
  {
    m_coloring = a_coloring;
    m_only     = None;

    this->clearSeenThrough();
  }
};

static_assert(std::is_trivially_copyable_v<RegionSettings>,
              "RegionSettings must be a plain value, for a kernel to take it");

} // namespace EBGeometry

#endif
