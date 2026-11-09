// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_Subdomains.hpp
 * @brief  Subdomains: the parts of a domain, each a set of regions, and the role of each surface for the domain.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_SUBDOMAINS_HPP
#define EBGEOMETRY_SUBDOMAINS_HPP

// Std includes
#include <algorithm>
#include <cstdint>
#include <vector>

// Our includes
#include "EBGeometry_Array.hpp"
#include "EBGeometry_GPU.hpp"
#include "EBGeometry_Macros.hpp"
#include "EBGeometry_SurfaceRef.hpp"

namespace EBGeometry {

/**
 * @brief What a surface is to a domain, at a point, from the regions on its two sides (see Subdomains::role()).
 */
enum class SurfaceRole : uint8_t
{
  None,      ///< Neither side is in a subdomain: the solver does not cut it.
  Boundary,  ///< One side is in a subdomain: cut once, from that side; its surface label gives the boundary condition.
  Interface, ///< The two sides are in two different subdomains: one surface the solver couples across.
  Internal ///< Both sides are in the same subdomain (two of its regions touching): the solver decides; usually not cut.
};

/**
 * @brief The subdomains of a domain (see Domain), the regions a downstream code computes in: each a set of regions
 * (see Overlay()), usually one, coupled to the others through the interfaces between them.
 * @details Built on the host (addSubdomain()), then a plain value, trivially copyable, that a kernel takes like any
 * functor member: subdomainOf() and role() are callable on a device. A subdomain's implicit function is the smallest of its
 * regions' functions (Tape::signedDistance(p, subdomains, subdomain)), what a cut-cell solver cuts for it. Regions are held
 * as ranges, so a group of regions (an Overlay, ExprIndex::getRegionGroup()) is one; at most MaxRanges ranges in all.
 */
class Subdomains
{
public:
  /**
   * @brief No subdomain, or a region in no subdomain.
   */
  static constexpr uint32_t None = 0xFFFFFFFFU;

  /**
   * @brief The most ranges of regions the subdomains can hold together.
   */
  static constexpr uint32_t MaxRanges = 32;

  /**
   * @brief A range of regions in one subdomain.
   */
  struct Range
  {
    /**
     * @brief The first region.
     */
    uint32_t m_first = 0;

    /**
     * @brief How many regions.
     */
    uint32_t m_count = 0;

    /**
     * @brief The subdomain.
     */
    uint32_t m_subdomain = None;
  };

  /**
   * @brief Add a subdomain.
   * @details A region can be in one subdomain only; aborts with a message otherwise, or if the subdomains would hold more
   * than MaxRanges ranges, in every build.
   * @param[in] a_regions Its regions (not empty), in any order.
   * @return The subdomain's number, from 0 in the order added.
   */
  EBGEOMETRY_HOST
  uint32_t
  addSubdomain(std::vector<uint32_t> a_regions)
  {
    EBGEOMETRY_REQUIRE(!a_regions.empty(), "Subdomains: a subdomain needs at least one region");

    std::sort(a_regions.begin(), a_regions.end());

    a_regions.erase(std::unique(a_regions.begin(), a_regions.end()), a_regions.end());

    const uint32_t subdomain = m_numSubdomains;

    for (size_t i = 0; i < a_regions.size();) {
      EBGEOMETRY_REQUIRE(this->subdomainOf(a_regions[i]) == None,
                         "Subdomains: region %u is already in subdomain %u",
                         unsigned(a_regions[i]),
                         unsigned(this->subdomainOf(a_regions[i])));

      // A run of consecutive regions is one range.
      size_t end = i + 1;

      while (end < a_regions.size() && a_regions[end] == a_regions[end - 1] + 1U) {
        end++;
      }

      EBGEOMETRY_REQUIRE(
        m_numRanges < MaxRanges, "Subdomains: the subdomains hold more than %u ranges of regions", unsigned(MaxRanges));

      m_ranges[m_numRanges++] = Range{a_regions[i], static_cast<uint32_t>(end - i), subdomain};

      i = end;
    }

    return m_numSubdomains++;
  }

  /**
   * @brief How many subdomains there are.
   * @return The count.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline uint32_t
  getNumSubdomains() const noexcept
  {
    return m_numSubdomains;
  }

  /**
   * @brief How many ranges of regions the subdomains hold.
   * @return The count.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline uint32_t
  getNumRanges() const noexcept
  {
    return m_numRanges;
  }

  /**
   * @brief A range of regions.
   * @param[in] a_range Range (below getNumRanges()).
   * @return The range.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline const Range&
  getRange(const uint32_t a_range) const noexcept
  {
    EBGEOMETRY_EXPECT(a_range < m_numRanges);

    return m_ranges[a_range];
  }

  /**
   * @brief The subdomain a region is in.
   * @param[in] a_region Region.
   * @return The subdomain, or None.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline uint32_t
  subdomainOf(const uint32_t a_region) const noexcept
  {
    for (uint32_t i = 0; i < m_numRanges; i++) {
      if (a_region >= m_ranges[i].m_first && a_region - m_ranges[i].m_first < m_ranges[i].m_count) {
        return m_ranges[i].m_subdomain;
      }
    }

    return None;
  }

  /**
   * @brief What a surface is to the solver at a point: from the regions on its two sides there, as a tape's region
   * queries report them (Tape::closestSurface(p, region)).
   * @tparam T Floating-point precision.
   * @param[in] a_surface The surface, with its sides.
   * @return Its role; None when the sides are not set.
   */
  template <class T>
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline SurfaceRole
  role(const SurfaceRef<T>& a_surface) const noexcept
  {
    const uint32_t minus =
      (a_surface.m_ownerRegion != SurfaceRef<T>::None) ? this->subdomainOf(a_surface.m_ownerRegion) : None;
    const uint32_t plus =
      (a_surface.m_neighborRegion != SurfaceRef<T>::None) ? this->subdomainOf(a_surface.m_neighborRegion) : None;

    if (minus == None && plus == None) {
      return SurfaceRole::None;
    }
    if (minus == None || plus == None) {
      return SurfaceRole::Boundary;
    }

    return (minus == plus) ? SurfaceRole::Internal : SurfaceRole::Interface;
  }

private:
  /**
   * @brief The ranges.
   */
  Array<Range, MaxRanges> m_ranges{};

  /**
   * @brief How many ranges are used.
   */
  uint32_t m_numRanges = 0;

  /**
   * @brief How many subdomains there are.
   */
  uint32_t m_numSubdomains = 0;
};

} // namespace EBGeometry

#endif
