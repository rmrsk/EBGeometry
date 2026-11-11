// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_SurfaceGroups.hpp
 * @brief  SurfaceSelector and SurfaceGroups: which surfaces a downstream code groups under one meaning.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_SURFACEGROUPS_HPP
#define EBGEOMETRY_SURFACEGROUPS_HPP

// Std includes
#include <cstdint>
#include <type_traits>
#include <vector>

// Our includes
#include "EBGeometry_GPU.hpp"
#include "EBGeometry_LabelId.hpp"
#include "EBGeometry_Macros.hpp"
#include "EBGeometry_PODVector.hpp"
#include "EBGeometry_Pool.hpp"
#include "EBGeometry_SurfaceRef.hpp"

namespace EBGeometry {

/**
 * @brief Conditions on a point of a surface, all of which must hold; an unset one (Any) matches anything.
 * @details A piece (piece()) is a shape's surface between two regions, what one clicks; a part of it (withPart(),
 * a box's face, a mesh's face) is finer. Broader selectors leave conditions unset: a whole surface (whatever its part
 * and sides), every surface with a label, every surface between two regions (an interface, either order), every
 * surface of a region (either side), or between two materials. Patches with the same attributes are one piece: the
 * conditions say what a surface is, not where. The remaining surfaces (rest()) are what no other selector of a
 * domain's surface groups hold. Matched against a SurfaceRef with its sides set (a tape's region queries set them); a plain value,
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
   * @brief Whether the selector holds the remaining surfaces (rest()): surface groups try it after every other.
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
    selector.m_regionA = a_surface.m_ownerRegion;
    selector.m_regionB = a_surface.m_neighborRegion;

    return selector;
  }

  /**
   * @brief The remaining surfaces: every surface no other selector of the surface groups holds, whatever the order the
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
           sides(m_regionA, m_regionB, a_surface.m_ownerRegion, a_surface.m_neighborRegion) &&
           sides(m_materialA, m_materialB, a_surface.m_ownerMaterial.value(), a_surface.m_neighborMaterial.value());
  }
};

/**
 * @brief A domain's surface groups, read on the host or a device: each group a set of surfaces, given by its selectors
 * (SurfaceSelector), that the caller gives one meaning (a boundary condition, an exchange with another code), and the
 * group of a surface, so that a surface found at a point (Domain::closestSurface()) gives its group, and the group its
 * meaning.
 * @details A view of the selectors in a pool: trivially copyable, a pool handle (see PoolHandle). A Domain builds it
 * from a DomainSetup and returns it (Domain::surfaceGroups()); a renderer showing surface groups takes one built on its
 * own (DomainSetup::buildSurfaceGroups()). The pool must outlive it. A default-constructed one has no surface groups. A
 * surface is in the group of the first selector it matches, in the order they were added, the remaining surfaces'
 * (SurfaceSelector::rest()) tried last. The groups' names and meanings are the caller's: the class holds numbers only.
 */
class SurfaceGroups : public PoolHandle
{
public:
  /**
   * @brief No surface group: a surface no selector matches.
   */
  static constexpr uint32_t None = 0xFFFFFFFFU;

  /**
   * @brief The arrays, as offsets: what a Domain stores.
   */
  struct Layout
  {
    /**
     * @brief The selectors, in the order added.
     */
    PODVector<SurfaceSelector> m_selectors;

    /**
     * @brief Each selector's surface group.
     */
    PODVector<uint32_t> m_groupOf;

    /**
     * @brief How many surface groups there are.
     */
    uint32_t m_numSurfaceGroups = 0;
  };

  /**
   * @brief No surface groups.
   */
  SurfaceGroups() noexcept = default;

  /**
   * @brief Build the selectors in a pool.
   * @details Aborts with a message, in every build, when the two lists differ in length or a selector's surface group
   * is not below @p a_numSurfaceGroups.
   * @param[in,out] a_pool             Pool to reserve the arrays from (not frozen).
   * @param[in]     a_selectors        The selectors, in order.
   * @param[in]     a_surfaceGroups    Each selector's surface group.
   * @param[in]     a_numSurfaceGroups How many surface groups there are.
   */
  EBGEOMETRY_HOST
  SurfaceGroups(Pool&                               a_pool,
                const std::vector<SurfaceSelector>& a_selectors,
                const std::vector<uint32_t>&        a_surfaceGroups,
                const uint32_t                      a_numSurfaceGroups)
  {
    EBGEOMETRY_REQUIRE(a_selectors.size() == a_surfaceGroups.size(),
                       "SurfaceGroups: %zu selectors and %zu surface groups of them",
                       a_selectors.size(),
                       a_surfaceGroups.size());

    this->attach(a_pool, "SurfaceGroups");

    const uint32_t n = static_cast<uint32_t>(a_selectors.size());

    m_layout.m_selectors.reserveSizedFrom(a_pool, n);
    m_layout.m_groupOf.reserveSizedFrom(a_pool, n);
    m_layout.m_numSurfaceGroups = a_numSurfaceGroups;

    for (uint32_t i = 0; i < n; i++) {
      EBGEOMETRY_REQUIRE(a_surfaceGroups[i] < a_numSurfaceGroups,
                         "SurfaceGroups: selector %u is in surface group %u of %u",
                         unsigned(i),
                         unsigned(a_surfaceGroups[i]),
                         unsigned(a_numSurfaceGroups));

      m_layout.m_selectors.at(this->base(), i) = a_selectors[i];
      m_layout.m_groupOf.at(this->base(), i)   = a_surfaceGroups[i];
    }
  }

  /**
   * @brief Rebuild a view from a stored layout and the location of the pool that holds it (as a Domain does).
   * @param[in] a_layout   The arrays.
   * @param[in] a_location Where they live.
   */
  EBGEOMETRY_HOST_DEVICE
  SurfaceGroups(const Layout& a_layout, const PoolLocation& a_location) noexcept
    : PoolHandle(a_location), m_layout(a_layout)
  {}

  /**
   * @brief The arrays, as offsets.
   * @return The layout.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline const Layout&
  layout() const noexcept
  {
    return m_layout;
  }

  /**
   * @brief How many surface groups there are.
   * @return The count.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline uint32_t
  getNumSurfaceGroups() const noexcept
  {
    return m_layout.m_numSurfaceGroups;
  }

  /**
   * @brief How many selectors there are, over all surface groups.
   * @return The count.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline uint32_t
  getNumSelectors() const noexcept
  {
    return m_layout.m_selectors.size();
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
    return m_layout.m_selectors.at(this->base(), a_selector);
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
    return m_layout.m_groupOf.at(this->base(), a_selector);
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
    const uint32_t n = m_layout.m_selectors.size();

    if (n == 0U) {
      return None;
    }

    const SurfaceSelector* selectors = m_layout.m_selectors.data(this->base());
    const uint32_t*        groups    = m_layout.m_groupOf.data(this->base());

    // The remaining surfaces' selectors last.
    for (int pass = 0; pass < 2; pass++) {
      for (uint32_t i = 0; i < n; i++) {
        if (selectors[i].m_rest == (pass == 1) && selectors[i].matches(a_surface)) {
          return groups[i];
        }
      }
    }

    return None;
  }

  /**
   * @brief A view resolving against another pool of the selectors' mirror chain (see PoolHandle).
   * @param[in] a_pool The pool, or a mirror of it.
   * @return The view.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  inline SurfaceGroups
  rebasedView(const Pool& a_pool) const noexcept
  {
    SurfaceGroups view = *this;

    view.setLocation(this->rebasedLocation(a_pool, m_layout.m_groupOf.endByte(), "SurfaceGroups"));

    return view;
  }

private:
  /**
   * @brief The arrays.
   */
  Layout m_layout;
};

static_assert(std::is_trivially_copyable_v<SurfaceGroups>,
              "SurfaceGroups must be trivially copyable, for a kernel to take it");

} // namespace EBGeometry

#endif
