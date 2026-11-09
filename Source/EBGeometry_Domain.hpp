// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_Domain.hpp
 * @brief  Domain: a description of the regions a downstream code computes in and its tagged boundary conditions.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_DOMAIN_HPP
#define EBGEOMETRY_DOMAIN_HPP

// Std includes
#include <cstdint>
#include <type_traits>

// Our includes
#include "EBGeometry_GPU.hpp"
#include "EBGeometry_Phases.hpp"
#include "EBGeometry_Scheme.hpp"
#include "EBGeometry_SurfaceRef.hpp"

namespace EBGeometry {

/**
 * @brief A computational domain, as a description for a downstream code (a solver EBGeometry does not provide): the
 * regions it computes in (its phases) and its surface groups, each a boundary condition's tag (the surface group's
 * name, kept with RegionNames) and id (its number in the scheme).
 * @details A plain value, trivially copyable, that a kernel takes whole: at a cut cell, m_phases says what the
 * surface is to the domain (Phases::role()) and m_scheme its boundary condition (Scheme::surfaceGroupOf()). The
 * surfaces it must tag are those it touches, every surface with a side in one of its regions, the internal ones
 * between two of its regions included. Materials and their properties are the regions' (MaterialLabel()).
 */
struct Domain
{
  /**
   * @brief The regions it computes in, as phases.
   */
  Phases m_phases;

  /**
   * @brief Its surface groups.
   */
  Scheme m_scheme;

  /**
   * @brief Whether it computes in a region.
   * @param[in] a_region The region.
   * @return True if the region is in one of its phases.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline bool
  computesIn(const uint32_t a_region) const noexcept
  {
    return m_phases.phaseOf(a_region) != Phases::None;
  }

  /**
   * @brief Whether a surface between two regions is one it must give a condition: one side or both in its regions.
   * @param[in] a_first  A region on one side.
   * @param[in] a_second The region on the other.
   * @return True if it touches the domain's regions.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline bool
  touches(const uint32_t a_first, const uint32_t a_second) const noexcept
  {
    return this->computesIn(a_first) || this->computesIn(a_second);
  }
};

static_assert(std::is_trivially_copyable_v<Domain>, "Domain must be a plain value, for a kernel to take it");

} // namespace EBGeometry

#endif
