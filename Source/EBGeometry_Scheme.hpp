// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_Scheme.hpp
 * @brief  SurfaceSelector and Scheme: which surfaces a solver groups under one boundary condition.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_SCHEME_HPP
#define EBGEOMETRY_SCHEME_HPP

// Std includes
#include <cstdint>
#include <type_traits>

// Our includes
#include "EBGeometry_GPU.hpp"
#include "EBGeometry_LabelId.hpp"
#include "EBGeometry_Macros.hpp"
#include "EBGeometry_SurfaceRef.hpp"

namespace EBGeometry {

/**
 * @brief Conditions on a point of a surface, all of which must hold; an unset one (Any) matches anything.
 * @details A piece (piece()) is a shape's surface between two regions, what one clicks; a part of it (withPart(),
 * a box's face, a mesh's face) is finer. Broader selectors leave conditions unset: a whole surface (whatever its part
 * and sides), every surface with a label, every surface between two regions (an interface, either order), every
 * surface of a region (either side), or between two materials. Patches with the same attributes are one piece: the
 * conditions say what a surface is, not where. The remaining surfaces (rest()) are what no other selector of a
 * scheme holds. Matched against a SurfaceRef with its sides set (a tape's region queries set them); a plain value,
 * usable on a device.
 */
struct SurfaceSelector
{
  /**
   * @brief An unset condition: matches anything.
   */
  static constexpr uint32_t Any = 0xFFFFFFFFU;

  /**
   * @brief The surface (SurfaceRef::m_surface), or Any.
   */
  uint32_t m_surface = Any;

  /**
   * @brief The part (SurfaceRef::m_part), or Any.
   */
  uint32_t m_part = Any;

  /**
   * @brief A region on one side, or Any.
   */
  uint32_t m_regionA = Any;

  /**
   * @brief The region on the other side, or Any (with m_regionA set: m_regionA on either side, anything across).
   */
  uint32_t m_regionB = Any;

  /**
   * @brief The surface label (SurfaceRef::m_surfaceLabel), or Any.
   */
  uint32_t m_label = Any;

  /**
   * @brief A material on one side, or Any.
   */
  uint32_t m_materialA = Any;

  /**
   * @brief The material on the other side, or Any (as m_regionB).
   */
  uint32_t m_materialB = Any;

  /**
   * @brief Whether the selector holds the remaining surfaces (rest()): a scheme tries it after every other.
   */
  bool m_rest = false;

  /**
   * @brief The piece a surface point belongs to: its surface between the two regions on its sides, whatever the part
   * (withPart() narrows it to one).
   * @tparam T Floating-point precision.
   * @param[in] a_surface A surface with its sides (from a tape's region queries).
   * @return The selector.
   */
  template <class T>
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  static inline SurfaceSelector
  piece(const SurfaceRef<T>& a_surface) noexcept
  {
    SurfaceSelector selector;

    selector.m_surface = a_surface.m_surface;
    selector.m_regionA = a_surface.m_insideRegion;
    selector.m_regionB = a_surface.m_outsideRegion;

    return selector;
  }

  /**
   * @brief The remaining surfaces: every surface no other selector of the scheme holds, whatever the order the
   * selectors were added in.
   * @return The selector.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  static inline SurfaceSelector
  rest() noexcept
  {
    SurfaceSelector selector;

    selector.m_rest = true;

    return selector;
  }

  /**
   * @brief Every point of a surface, whatever its part and sides.
   * @param[in] a_surface The surface (SurfaceRef::m_surface).
   * @return The selector.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  static inline SurfaceSelector
  surface(const uint32_t a_surface) noexcept
  {
    SurfaceSelector selector;

    selector.m_surface = a_surface;

    return selector;
  }

  /**
   * @brief Every surface with a surface label.
   * @param[in] a_label The label.
   * @return The selector.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  static inline SurfaceSelector
  label(const SurfaceLabelId a_label) noexcept
  {
    SurfaceSelector selector;

    selector.m_label = a_label.value();

    return selector;
  }

  /**
   * @brief Every surface between two regions, either way round: an interface.
   * @param[in] a_first  A region.
   * @param[in] a_second The other region.
   * @return The selector.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  static inline SurfaceSelector
  interface(const uint32_t a_first, const uint32_t a_second) noexcept
  {
    SurfaceSelector selector;

    selector.m_regionA = a_first;
    selector.m_regionB = a_second;

    return selector;
  }

  /**
   * @brief Every surface of a region: those with the region on either side.
   * @param[in] a_region The region.
   * @return The selector.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  static inline SurfaceSelector
  region(const uint32_t a_region) noexcept
  {
    SurfaceSelector selector;

    selector.m_regionA = a_region;

    return selector;
  }

  /**
   * @brief Every surface between two materials, either way round (one material alone: on either side).
   * @param[in] a_first  A material.
   * @param[in] a_second The other material, or none for any.
   * @return The selector.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  static inline SurfaceSelector
  materials(const MaterialLabelId a_first, const MaterialLabelId a_second = MaterialLabelId::none()) noexcept
  {
    SurfaceSelector selector;

    selector.m_materialA = a_first.value();
    selector.m_materialB = a_second.value();

    return selector;
  }

  /**
   * @brief The same selector restricted to one part (a cylinder's cap, a box's face).
   * @param[in] a_part The part.
   * @return The selector.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline SurfaceSelector
  withPart(const uint32_t a_part) const noexcept
  {
    SurfaceSelector selector = *this;

    selector.m_part = a_part;

    return selector;
  }

  /**
   * @brief Whether a surface point matches every condition set.
   * @tparam T Floating-point precision.
   * @param[in] a_surface The surface, with its sides and their materials set.
   * @return True if it matches.
   */
  template <class T>
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline bool
  matches(const SurfaceRef<T>& a_surface) const noexcept
  {
    const auto is = [](const uint32_t a_condition, const uint32_t a_value) {
      return a_condition == Any || a_condition == a_value;
    };

    // A pair of conditions on the two sides, either way round; one alone: on either side.
    const auto sides =
      [&is](const uint32_t a_first, const uint32_t a_second, const uint32_t a_minus, const uint32_t a_plus) {
        if (a_first == Any && a_second == Any) {
          return true;
        }
        if (a_second == Any) {
          return a_first == a_minus || a_first == a_plus;
        }

        return (is(a_first, a_minus) && is(a_second, a_plus)) || (is(a_first, a_plus) && is(a_second, a_minus));
      };

    return is(m_surface, a_surface.m_surface) && is(m_part, a_surface.m_part) &&
           is(m_label, a_surface.m_surfaceLabel.value()) &&
           sides(m_regionA, m_regionB, a_surface.m_insideRegion, a_surface.m_outsideRegion) &&
           sides(m_materialA, m_materialB, a_surface.m_insideMaterial.value(), a_surface.m_outsideMaterial.value());
  }
};

/**
 * @brief One solver's surface groups: "no-slip" for a flow solver, "dirichlet" and "jump" for an electrostatic
 * one, each a list of selectors (SurfaceSelector), so that a cut cell's surface (Tape::closestSurface(p, subdomains,
 * subdomain)) gives its surface group, and the surface group its boundary condition.
 * @details At most MaxSurfaceGroups surface groups and MaxSelectors selectors in all, so that a scheme is a plain value a kernel
 * takes (surfaceGroupOf() is callable on a device). A surface is in the surface group of the first selector it matches, in the
 * order they were added, the remaining surfaces' (SurfaceSelector::rest()) tried last. The surface groups' names are the caller's (a scheme holds numbers only).
 */
class Scheme
{
public:
  /**
   * @brief No surface group: a surface no selector matches.
   */
  static constexpr uint32_t None = 0xFFFFFFFFU;

  /**
   * @brief The most surface groups a scheme holds.
   */
  static constexpr uint32_t MaxSurfaceGroups = 8;

  /**
   * @brief The most selectors a scheme holds, over all its surface groups.
   */
  static constexpr uint32_t MaxSelectors = 32;

  /**
   * @brief Add a surface group.
   * @details Aborts with a message, in every build, beyond MaxSurfaceGroups.
   * @return Its number, from 0 in the order added.
   */
  EBGEOMETRY_HOST
  uint32_t
  addSurfaceGroup()
  {
    EBGEOMETRY_REQUIRE(m_numSurfaceGroups < MaxSurfaceGroups,
                       "Scheme: a scheme holds at most %u surface groups",
                       unsigned(MaxSurfaceGroups));

    return m_numSurfaceGroups++;
  }

  /**
   * @brief Add a selector to a surface group.
   * @details Aborts with a message, in every build, for a surface group not added or beyond MaxSelectors.
   * @param[in] a_group    The surface group.
   * @param[in] a_selector The selector.
   */
  EBGEOMETRY_HOST
  void
  add(const uint32_t a_group, const SurfaceSelector& a_selector)
  {
    EBGEOMETRY_REQUIRE(
      a_group < m_numSurfaceGroups, "Scheme: surface group %u of %u", unsigned(a_group), unsigned(m_numSurfaceGroups));
    EBGEOMETRY_REQUIRE(
      m_numSelectors < MaxSelectors, "Scheme: a scheme holds at most %u selectors", unsigned(MaxSelectors));

    m_selectors[m_numSelectors]     = a_selector;
    m_surfaceGroups[m_numSelectors] = a_group;

    m_numSelectors++;
  }

  /**
   * @brief Remove a surface group and its selectors; the surface groups after it move down by one.
   * @details Aborts with a message, in every build, for a surface group not added.
   * @param[in] a_group The surface group.
   */
  EBGEOMETRY_HOST
  void
  removeSurfaceGroup(const uint32_t a_group)
  {
    EBGEOMETRY_REQUIRE(
      a_group < m_numSurfaceGroups, "Scheme: surface group %u of %u", unsigned(a_group), unsigned(m_numSurfaceGroups));

    uint32_t kept = 0;

    for (uint32_t i = 0; i < m_numSelectors; i++) {
      if (m_surfaceGroups[i] != a_group) {
        m_selectors[kept]     = m_selectors[i];
        m_surfaceGroups[kept] = (m_surfaceGroups[i] > a_group) ? m_surfaceGroups[i] - 1U : m_surfaceGroups[i];

        kept++;
      }
    }

    m_numSelectors = kept;

    m_numSurfaceGroups--;
  }

  /**
   * @brief Remove the selectors of a surface group that equal one (to undo a click).
   * @param[in] a_group    The surface group.
   * @param[in] a_selector The selector.
   */
  EBGEOMETRY_HOST
  void
  remove(const uint32_t a_group, const SurfaceSelector& a_selector)
  {
    uint32_t kept = 0;

    for (uint32_t i = 0; i < m_numSelectors; i++) {
      const SurfaceSelector& s = m_selectors[i];

      const bool same = m_surfaceGroups[i] == a_group && s.m_surface == a_selector.m_surface &&
                        s.m_part == a_selector.m_part && s.m_regionA == a_selector.m_regionA &&
                        s.m_regionB == a_selector.m_regionB && s.m_label == a_selector.m_label &&
                        s.m_materialA == a_selector.m_materialA && s.m_materialB == a_selector.m_materialB &&
                        s.m_rest == a_selector.m_rest;

      if (!same) {
        m_selectors[kept]     = m_selectors[i];
        m_surfaceGroups[kept] = m_surfaceGroups[i];

        kept++;
      }
    }

    m_numSelectors = kept;
  }

  /**
   * @brief How many surface groups there are.
   * @return The count.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline uint32_t
  getNumSurfaceGroups() const noexcept
  {
    return m_numSurfaceGroups;
  }

  /**
   * @brief How many selectors there are, over all surface groups.
   * @return The count.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline uint32_t
  getNumSelectors() const noexcept
  {
    return m_numSelectors;
  }

  /**
   * @brief A selector.
   * @param[in] a_selector Its number (below getNumSelectors()).
   * @return The selector.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline const SurfaceSelector&
  getSelector(const uint32_t a_selector) const noexcept
  {
    EBGEOMETRY_EXPECT(a_selector < m_numSelectors);

    return m_selectors[a_selector];
  }

  /**
   * @brief The surface group of a selector.
   * @param[in] a_selector Its number (below getNumSelectors()).
   * @return The surface group.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline uint32_t
  getSurfaceGroupOfSelector(const uint32_t a_selector) const noexcept
  {
    EBGEOMETRY_EXPECT(a_selector < m_numSelectors);

    return m_surfaceGroups[a_selector];
  }

  /**
   * @brief The surface group a surface point is in: that of the first selector it matches, those of the remaining
   * surfaces (SurfaceSelector::rest()) tried last.
   * @tparam T Floating-point precision.
   * @param[in] a_surface The surface, with its sides and their materials (Tape::closestSurface(p, subdomains, subdomain),
   * firstRegionChange()).
   * @return The surface group, or None.
   */
  template <class T>
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline uint32_t
  surfaceGroupOf(const SurfaceRef<T>& a_surface) const noexcept
  {
    // The remaining surfaces' selectors last.
    for (int pass = 0; pass < 2; pass++) {
      for (uint32_t i = 0; i < m_numSelectors; i++) {
        if (m_selectors[i].m_rest == (pass == 1) && m_selectors[i].matches(a_surface)) {
          return m_surfaceGroups[i];
        }
      }
    }

    return None;
  }

private:
  /**
   * @brief The selectors.
   */
  SurfaceSelector m_selectors[MaxSelectors] = {};

  /**
   * @brief Each selector's surface group.
   */
  uint32_t m_surfaceGroups[MaxSelectors] = {};

  /**
   * @brief How many selectors are used.
   */
  uint32_t m_numSelectors = 0;

  /**
   * @brief How many surface groups there are.
   */
  uint32_t m_numSurfaceGroups = 0;
};

static_assert(std::is_trivially_copyable_v<Scheme>, "Scheme must be a plain value, for a kernel to take it");

} // namespace EBGeometry

#endif
