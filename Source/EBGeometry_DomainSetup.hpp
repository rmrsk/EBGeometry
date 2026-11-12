// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_DomainSetup.hpp
 * @brief  DomainSetup: how a domain is made, as an editable value on the host (see Domain).
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_DOMAINSETUP_HPP
#define EBGEOMETRY_DOMAINSETUP_HPP

// Std includes
#include <algorithm>
#include <cstdint>
#include <vector>

// Our includes
#include "EBGeometry_Macros.hpp"
#include "EBGeometry_Math.hpp"
#include "EBGeometry_Pool.hpp"
#include "EBGeometry_Subdomains.hpp"
#include "EBGeometry_SurfaceGroups.hpp"
#include "EBGeometry_SurfaceRef.hpp"

namespace EBGeometry {

/**
 * @brief How a domain is made: its subdomains (the regions each computes in) and its surface groups (the selectors of
 * each), as a plain value on the host, to build, copy, compare and edit.
 * @details A description, not the domain itself: a Domain is built from it for one tape (Domain(pool, tape, setup)),
 * as a snapshot, and editing the setup afterwards changes no domain already built. There is no limit on subdomains,
 * surface groups or selectors. The queries here (subdomainOf(), role(), surfaceGroupOf()) answer on the host what the
 * built domain answers on the host or a device, for tools that edit a setup (a viewer's panel, a coverage check).
 * Host only.
 */
class DomainSetup
{
public:
  /**
   * @brief Add a subdomain.
   * @details A region can be in one subdomain only; aborts with a message otherwise, or for an empty list, in every
   * build. A region listed twice counts once. Whether the regions are the tape's is checked when a Domain is built; the
   * setup keeps a row per region up to the largest it is given, so the numbers should be the tape's (Tape::region(),
   * ExprIndex), not arbitrary ones.
   * @param[in] a_regions Its regions, in any order.
   * @return The subdomain's number (its label in the domain), from 0 in the order added.
   */
  EBGEOMETRY_HOST
  uint32_t
  addSubdomain(std::vector<uint32_t> a_regions)
  {
    EBGEOMETRY_REQUIRE(!a_regions.empty(), "DomainSetup: a subdomain needs at least one region");

    std::sort(a_regions.begin(), a_regions.end());

    a_regions.erase(std::unique(a_regions.begin(), a_regions.end()), a_regions.end());

    for (const uint32_t r : a_regions) {
      EBGEOMETRY_REQUIRE(r != Subdomains::None, "DomainSetup: no region number %u", unsigned(r));
      EBGEOMETRY_REQUIRE(this->subdomainOf(r) == Subdomains::None,
                         "DomainSetup: region %u is already in subdomain %u",
                         unsigned(r),
                         unsigned(this->subdomainOf(r)));
    }

    const uint32_t subdomain = m_numSubdomains++;

    for (const uint32_t r : a_regions) {
      if (r >= m_subdomainOf.size()) {
        m_subdomainOf.resize(r + 1U, Subdomains::None);
      }

      m_subdomainOf[r] = subdomain;
    }

    return subdomain;
  }

  /**
   * @brief How many subdomains there are.
   * @return The count.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  uint32_t
  getNumSubdomains() const noexcept
  {
    return m_numSubdomains;
  }

  /**
   * @brief The subdomain a region is in.
   * @param[in] a_region The region.
   * @return The subdomain, or Subdomains::None.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  uint32_t
  subdomainOf(const uint32_t a_region) const noexcept
  {
    return (a_region < m_subdomainOf.size()) ? m_subdomainOf[a_region] : Subdomains::None;
  }

  /**
   * @brief Whether the domain computes in a region.
   * @param[in] a_region The region.
   * @return True if the region is in one of its subdomains.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  bool
  computesIn(const uint32_t a_region) const noexcept
  {
    return this->subdomainOf(a_region) != Subdomains::None;
  }

  /**
   * @brief Whether a surface between two regions is one the domain must give a condition: one side or both in its
   * regions.
   * @param[in] a_first  A region on one side.
   * @param[in] a_second The region on the other.
   * @return True if it touches the domain's regions.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  bool
  touches(const uint32_t a_first, const uint32_t a_second) const noexcept
  {
    return this->computesIn(a_first) || this->computesIn(a_second);
  }

  /**
   * @brief The regions of a subdomain.
   * @param[in] a_subdomain The subdomain (below getNumSubdomains()).
   * @return Its regions, in increasing order.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  std::vector<uint32_t>
  getRegions(const uint32_t a_subdomain) const
  {
    std::vector<uint32_t> regions;

    for (uint32_t r = 0; r < m_subdomainOf.size(); r++) {
      if (m_subdomainOf[r] == a_subdomain) {
        regions.push_back(r);
      }
    }

    return regions;
  }

  /**
   * @brief One past the largest region in a subdomain: the fewest regions a tape must have for the setup.
   * @return The count.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  uint32_t
  getRegionsNeeded() const noexcept
  {
    return static_cast<uint32_t>(m_subdomainOf.size());
  }

  /**
   * @brief What a surface is to the subdomains (as Subdomains::role()).
   * @tparam T Floating-point precision.
   * @param[in] a_surface The surface, with its sides.
   * @return Its role.
   */
  template <class T>
  [[nodiscard]] EBGEOMETRY_HOST
  SurfaceRole
  role(const SurfaceRef<T>& a_surface) const noexcept
  {
    const uint32_t minus =
      (a_surface.m_ownerRegion != SurfaceRef<T>::None) ? this->subdomainOf(a_surface.m_ownerRegion) : Subdomains::None;
    const uint32_t plus = (a_surface.m_neighborRegion != SurfaceRef<T>::None)
                            ? this->subdomainOf(a_surface.m_neighborRegion)
                            : Subdomains::None;

    if (minus == Subdomains::None && plus == Subdomains::None) {
      return SurfaceRole::None;
    }
    if (minus == Subdomains::None || plus == Subdomains::None) {
      return SurfaceRole::Boundary;
    }

    return (minus == plus) ? SurfaceRole::Internal : SurfaceRole::Interface;
  }

  /**
   * @brief Add a surface group.
   * @return Its number, from 0 in the order added.
   */
  EBGEOMETRY_HOST
  uint32_t
  addSurfaceGroup() noexcept
  {
    return m_numSurfaceGroups++;
  }

  /**
   * @brief Add a selector to a surface group.
   * @details Aborts with a message, in every build, for a surface group not added.
   * @param[in] a_group    The surface group.
   * @param[in] a_selector The selector.
   */
  EBGEOMETRY_HOST
  void
  add(const uint32_t a_group, const SurfaceSelector& a_selector)
  {
    EBGEOMETRY_REQUIRE(a_group < m_numSurfaceGroups,
                       "DomainSetup: surface group %u of %u",
                       unsigned(a_group),
                       unsigned(m_numSurfaceGroups));

    m_selectors.push_back(a_selector);
    m_surfaceGroups.push_back(a_group);
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
    EBGEOMETRY_REQUIRE(a_group < m_numSurfaceGroups,
                       "DomainSetup: surface group %u of %u",
                       unsigned(a_group),
                       unsigned(m_numSurfaceGroups));

    std::size_t kept = 0;

    for (std::size_t i = 0; i < m_selectors.size(); i++) {
      if (m_surfaceGroups[i] != a_group) {
        m_selectors[kept]     = m_selectors[i];
        m_surfaceGroups[kept] = (m_surfaceGroups[i] > a_group) ? m_surfaceGroups[i] - 1U : m_surfaceGroups[i];

        kept++;
      }
    }

    m_selectors.resize(kept);
    m_surfaceGroups.resize(kept);

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
    std::size_t kept = 0;

    for (std::size_t i = 0; i < m_selectors.size(); i++) {
      if (m_surfaceGroups[i] != a_group || !same(m_selectors[i], a_selector)) {
        m_selectors[kept]     = m_selectors[i];
        m_surfaceGroups[kept] = m_surfaceGroups[i];

        kept++;
      }
    }

    m_selectors.resize(kept);
    m_surfaceGroups.resize(kept);
  }

  /**
   * @brief How many surface groups there are.
   * @return The count.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  uint32_t
  getNumSurfaceGroups() const noexcept
  {
    return m_numSurfaceGroups;
  }

  /**
   * @brief How many selectors there are, over all surface groups.
   * @return The count.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  uint32_t
  getNumSelectors() const noexcept
  {
    return static_cast<uint32_t>(m_selectors.size());
  }

  /**
   * @brief A selector.
   * @param[in] a_selector Its number (below getNumSelectors()).
   * @return The selector.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  const SurfaceSelector&
  getSelector(const uint32_t a_selector) const
  {
    return m_selectors.at(a_selector);
  }

  /**
   * @brief The surface group of a selector.
   * @param[in] a_selector Its number (below getNumSelectors()).
   * @return The surface group.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  uint32_t
  getSurfaceGroupOfSelector(const uint32_t a_selector) const
  {
    return m_surfaceGroups.at(a_selector);
  }

  /**
   * @brief The surface group a surface point is in (as SurfaceGroups::surfaceGroupOf()).
   * @tparam T Floating-point precision.
   * @param[in] a_surface The surface, with its sides and their materials.
   * @return The surface group, or SurfaceGroups::None.
   */
  template <class T>
  [[nodiscard]] EBGEOMETRY_HOST
  uint32_t
  surfaceGroupOf(const SurfaceRef<T>& a_surface) const noexcept
  {
    for (int pass = 0; pass < 2; pass++) {
      for (std::size_t i = 0; i < m_selectors.size(); i++) {
        if (m_selectors[i].m_rest == (pass == 1) && m_selectors[i].matches(a_surface)) {
          return m_surfaceGroups[i];
        }
      }
    }

    return SurfaceGroups::None;
  }

  /**
   * @brief Build the subdomains for a tape of a number of regions.
   * @details Aborts with a message, in every build, when a subdomain holds a region the tape does not have.
   * @param[in,out] a_pool       Pool to build the table in (not frozen).
   * @param[in]     a_numRegions The tape's regions (Tape::getNumRegions()).
   * @return The subdomains.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  Subdomains
  buildSubdomains(Pool& a_pool, const uint32_t a_numRegions) const
  {
    EBGEOMETRY_REQUIRE(this->getRegionsNeeded() <= a_numRegions,
                       "DomainSetup: a subdomain holds region %u, the tape has %u regions",
                       unsigned(this->getRegionsNeeded() - 1U),
                       unsigned(a_numRegions));

    std::vector<uint32_t> table(a_numRegions, Subdomains::None);

    std::copy(m_subdomainOf.begin(), m_subdomainOf.end(), table.begin());

    return Subdomains(a_pool, table, m_numSubdomains);
  }

  /**
   * @brief Build the surface groups.
   * @param[in,out] a_pool Pool to build the selectors in (not frozen).
   * @return The surface groups.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  SurfaceGroups
  buildSurfaceGroups(Pool& a_pool) const
  {
    return SurfaceGroups(a_pool, m_selectors, m_surfaceGroups, m_numSurfaceGroups);
  }

  /**
   * @brief Whether two setups make the same domain: the same subdomains, and the same selectors in the same groups and
   * order.
   * @param[in] a_other The other setup.
   * @return True if they do.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  bool
  operator==(const DomainSetup& a_other) const noexcept
  {
    if (m_numSubdomains != a_other.m_numSubdomains || m_numSurfaceGroups != a_other.m_numSurfaceGroups ||
        m_selectors.size() != a_other.m_selectors.size() || m_surfaceGroups != a_other.m_surfaceGroups) {
      return false;
    }

    const std::size_t regions = Math::max(m_subdomainOf.size(), a_other.m_subdomainOf.size());

    for (std::size_t r = 0; r < regions; r++) {
      if (this->subdomainOf(static_cast<uint32_t>(r)) != a_other.subdomainOf(static_cast<uint32_t>(r))) {
        return false;
      }
    }

    for (std::size_t i = 0; i < m_selectors.size(); i++) {
      if (!same(m_selectors[i], a_other.m_selectors[i])) {
        return false;
      }
    }

    return true;
  }

  /**
   * @brief Whether two setups make different domains.
   * @param[in] a_other The other setup.
   * @return True if they do.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  bool
  operator!=(const DomainSetup& a_other) const noexcept
  {
    return !(*this == a_other);
  }

private:
  /**
   * @brief Whether two selectors set the same conditions.
   * @param[in] a_first  A selector.
   * @param[in] a_second Another.
   * @return True if they do.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  static bool
  same(const SurfaceSelector& a_first, const SurfaceSelector& a_second) noexcept
  {
    return a_first.m_surface == a_second.m_surface && a_first.m_part == a_second.m_part &&
           a_first.m_regionA == a_second.m_regionA && a_first.m_regionB == a_second.m_regionB &&
           a_first.m_label == a_second.m_label && a_first.m_materialA == a_second.m_materialA &&
           a_first.m_materialB == a_second.m_materialB && a_first.m_rest == a_second.m_rest;
  }

  /**
   * @brief The subdomain of each region, Subdomains::None for a region in none, up to the largest region in one.
   */
  std::vector<uint32_t> m_subdomainOf;

  /**
   * @brief How many subdomains there are.
   */
  uint32_t m_numSubdomains = 0;

  /**
   * @brief The selectors, in the order added.
   */
  std::vector<SurfaceSelector> m_selectors;

  /**
   * @brief Each selector's surface group.
   */
  std::vector<uint32_t> m_surfaceGroups;

  /**
   * @brief How many surface groups there are.
   */
  uint32_t m_numSurfaceGroups = 0;
};

} // namespace EBGeometry

#endif
