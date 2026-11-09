// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_Phases.hpp
 * @brief  Phases: the regions a solver computes in, and the role of each surface for it.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_PHASES_HPP
#define EBGEOMETRY_PHASES_HPP

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
 * @brief What a surface is to a solver, at a point, from the regions on its two sides (see Phases::role()).
 */
enum class SurfaceRole : uint8_t
{
  None,      ///< Neither side is in a phase: the solver does not cut it.
  Boundary,  ///< One side is in a phase: cut once, from that side; its surface label gives the boundary condition.
  Interface, ///< The two sides are in two different phases: one surface the solver couples across.
  Internal   ///< Both sides are in the same phase (two of its regions touching): the solver decides; usually not cut.
};

/**
 * @brief The phases a solver computes in: each a set of regions (see Overlay()), usually one.
 * @details Built on the host (addPhase()), then a plain value, trivially copyable, that a kernel takes like any
 * functor member: phaseOf() and role() are callable on a device. A phase's implicit function is the smallest of its
 * regions' functions (Tape::signedDistance(p, phases, phase)), what a cut-cell solver cuts for it. Regions are held
 * as ranges, so a group of regions (an Overlay, ExprIndex::getGroup()) is one; at most MaxRanges ranges in all.
 */
class Phases
{
public:
  /**
   * @brief No phase, or a region in no phase.
   */
  static constexpr uint32_t None = 0xFFFFFFFFU;

  /**
   * @brief The most ranges of regions the phases can hold together.
   */
  static constexpr uint32_t MaxRanges = 32;

  /**
   * @brief A range of regions in one phase.
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
     * @brief The phase.
     */
    uint32_t m_phase = None;
  };

  /**
   * @brief Add a phase.
   * @details A region can be in one phase only; aborts with a message otherwise, or if the phases would hold more
   * than MaxRanges ranges, in every build.
   * @param[in] a_regions Its regions (not empty), in any order.
   * @return The phase's number, from 0 in the order added.
   */
  EBGEOMETRY_HOST
  uint32_t
  addPhase(std::vector<uint32_t> a_regions)
  {
    EBGEOMETRY_REQUIRE(!a_regions.empty(), "Phases: a phase needs at least one region");

    std::sort(a_regions.begin(), a_regions.end());

    a_regions.erase(std::unique(a_regions.begin(), a_regions.end()), a_regions.end());

    const uint32_t phase = m_numPhases;

    for (size_t i = 0; i < a_regions.size();) {
      EBGEOMETRY_REQUIRE(this->phaseOf(a_regions[i]) == None,
                         "Phases: region %u is already in phase %u",
                         unsigned(a_regions[i]),
                         unsigned(this->phaseOf(a_regions[i])));

      // A run of consecutive regions is one range.
      size_t end = i + 1;

      while (end < a_regions.size() && a_regions[end] == a_regions[end - 1] + 1U) {
        end++;
      }

      EBGEOMETRY_REQUIRE(
        m_numRanges < MaxRanges, "Phases: the phases hold more than %u ranges of regions", unsigned(MaxRanges));

      m_ranges[m_numRanges++] = Range{a_regions[i], static_cast<uint32_t>(end - i), phase};

      i = end;
    }

    return m_numPhases++;
  }

  /**
   * @brief How many phases there are.
   * @return The count.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline uint32_t
  getNumPhases() const noexcept
  {
    return m_numPhases;
  }

  /**
   * @brief How many ranges of regions the phases hold.
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
   * @brief The phase a region is in.
   * @param[in] a_region Region.
   * @return The phase, or None.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline uint32_t
  phaseOf(const uint32_t a_region) const noexcept
  {
    for (uint32_t i = 0; i < m_numRanges; i++) {
      if (a_region >= m_ranges[i].m_first && a_region - m_ranges[i].m_first < m_ranges[i].m_count) {
        return m_ranges[i].m_phase;
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
      (a_surface.m_minusRegion != SurfaceRef<T>::None) ? this->phaseOf(a_surface.m_minusRegion) : None;
    const uint32_t plus =
      (a_surface.m_plusRegion != SurfaceRef<T>::None) ? this->phaseOf(a_surface.m_plusRegion) : None;

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
   * @brief How many phases there are.
   */
  uint32_t m_numPhases = 0;
};

} // namespace EBGeometry

#endif
